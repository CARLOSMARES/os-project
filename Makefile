# --- Makefile robusto para OS x86_64 custom con string.c ---

# Herramientas
NASM = nasm

# Preferir toolchain cruzado i686 (x86 32-bit)
GCC := $(shell which i686-elf-gcc 2>/dev/null)
LD_CROSS := $(shell which i686-elf-ld 2>/dev/null)
OBJCOPY_CROSS := $(shell which i686-elf-objcopy 2>/dev/null)

ifeq ($(GCC),)
  GCC = gcc
  CFLAGS_ARCH = -m32
  $(info [!] i686-elf-gcc no encontrado; usando gcc del sistema con -m32)
else
  CFLAGS_ARCH =
endif

ifeq ($(LD_CROSS),)
  LD = ld
  LDFLAGS_ARCH = -m elf_i386
else
  LD = $(LD_CROSS)
  LDFLAGS_ARCH =
endif

ifeq ($(OBJCOPY_CROSS),)
  OBJCOPY = objcopy
else
  OBJCOPY = $(OBJCOPY_CROSS)
endif

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
FS_DIR = fs
INIT_DIR = init
INCLUDE_DIR = include
BUILD_DIR = build
OUTPUT_DIR = output

BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_LD = $(KERNEL_DIR)/kernel.ld

# Archivos fuente
FS_C_FILES = $(wildcard $(FS_DIR)/*.c)
INIT_C_FILES = $(wildcard $(INIT_DIR)/*.c)
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c) $(KERNEL_DIR)/string.c

# Archivos de salida
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_O_FILES = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel_%.o,$(KERNEL_C_FILES))
FS_O_FILES = $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/fs_%.o,$(FS_C_FILES))
INIT_O_FILES = $(patsubst $(INIT_DIR)/%.c,$(BUILD_DIR)/init_%.o,$(INIT_C_FILES))
FLOPPY_IMG = $(OUTPUT_DIR)/floppy.img
KERNEL_SECTORS_H = $(BUILD_DIR)/kernel_sectors.inc

# Flags
CFLAGS = -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Os -fdata-sections -ffunction-sections \
         -fno-pic -I$(INCLUDE_DIR) $(CFLAGS_ARCH) -MMD -MP
LDFLAGS = -n -nostdlib -T $(KERNEL_LD) --gc-sections $(LDFLAGS_ARCH)

# Objetivo principal
all: check-toolchain $(FLOPPY_IMG)

# --- Reglas de construcción ---

check-toolchain:
	@if ! $(GCC) $(CFLAGS) -v >/dev/null 2>&1; then \
	  echo "[!] Toolchain no soporta -m32. Instala i686-elf-gcc o gcc-multilib"; \
	  exit 1; \
	fi

# Boot sector (512 bytes)
$(BOOT_BIN): $(BOOT_ASM) $(KERNEL_BIN) $(KERNEL_SECTORS_H)
	@echo "[+] Compilando boot sector..."
	mkdir -p $(BUILD_DIR)
	$(NASM) -f bin -I $(BUILD_DIR)/ $< -o $@

# Kernel binario plano + kernel_sectors.inc
$(KERNEL_BIN): $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES)
	@echo "[+] Enlazando kernel..."
	mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $(BUILD_DIR)/kernel_temp.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel_temp.elf $@
	rm $(BUILD_DIR)/kernel_temp.elf
	$(MAKE) $(KERNEL_SECTORS_H)

# kernel_sectors.inc
$(KERNEL_SECTORS_H): $(KERNEL_BIN)
	@echo "[+] Generando kernel_sectors.inc..."
	actual_size=$$(stat -c %s $(KERNEL_BIN)); \
	mod512=$$(( $$actual_size % 512 )); \
	if [ $$mod512 -ne 0 ]; then \
		pad_size=$$((512 - $$mod512)); \
		dd if=/dev/zero bs=1 count=$$pad_size >> $(KERNEL_BIN) 2>/dev/null; \
		actual_size=$$(( $$actual_size + $$pad_size )); \
	fi; \
	kernel_sectors=$$(( ($$actual_size + 511) / 512 )); \
	if [ $$kernel_sectors -lt 1 ]; then kernel_sectors=1; fi; \
	echo "[i] kernel.bin ocupa $$kernel_sectors sectores (512 bytes)"; \
	echo "%define KERNEL_SECTORS $$kernel_sectors" > $@

# Objetos init
$(BUILD_DIR)/init_%.o: $(INIT_DIR)/%.c
	@echo "[+] Compilando init $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Objetos kernel
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c
	@echo "[+] Compilando kernel $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Objetos fs
$(BUILD_DIR)/fs_%.o: $(FS_DIR)/%.c
	@echo "[+] Compilando fs $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Imagen de disquete 1.44MB
$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "[+] Construyendo imagen de disquete 1.44MB..."
	mkdir -p $(OUTPUT_DIR)
	dd if=/dev/zero of=$@ bs=1024 count=1440 status=none
	dd if=$(BOOT_BIN) of=$@ bs=512 count=1 conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc status=none

# --- Limpieza ---
clean-build:
	@echo "[•] Limpiando build..."
	rm -rf $(BUILD_DIR)

clean-all: clean-build
	@echo "[•] Limpiando todo..."
	rm -rf $(OUTPUT_DIR)

# --- Ejecutar en QEMU ---
run: $(FLOPPY_IMG)
	qemu-system-x86_64 -drive format=raw,file=$(FLOPPY_IMG),if=floppy -boot a -m 128M -accel tcg

debug: $(FLOPPY_IMG)
	qemu-system-x86_64 -s -S -drive format=raw,file=$(FLOPPY_IMG),if=floppy -boot a -m 128M -accel tcg

# Incluir dependencias auto-generadas (.d)
-include $(BUILD_DIR)/*.d

.PHONY: all clean-build clean-all run debug check-toolchain
