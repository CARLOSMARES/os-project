# --- Makefile para OS x86_64 con subdirectorios y disquete ---

# Toolchain
CROSS   = i686-elf-
CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy
NASM    = nasm

ifeq (, $(shell which $(CC)))
    $(warning [!] i686-elf-gcc no encontrado; usando gcc del sistema con -m32)
    CC = gcc
    LD = ld
    OBJCOPY = objcopy
    CFLAGS_ARCH = -m32
    LDFLAGS_ARCH = -m elf_i386
else
    CFLAGS_ARCH =
    LDFLAGS_ARCH =
endif

# Flags de compilación
CFLAGS  = -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -Iinclude $(CFLAGS_ARCH)
LDFLAGS = -n -nostdlib -T kernel/kernel.ld --gc-sections $(LDFLAGS_ARCH)

# Directorios
KERNEL_DIR = kernel
FS_DIR     = fs
INIT_DIR   = init
BOOT_DIR   = boot
BUILD_DIR  = build
OUTPUT_DIR = output

# Archivos fuente
KERNEL_SRC = $(wildcard $(KERNEL_DIR)/*.c) \
             $(wildcard $(KERNEL_DIR)/lib/*.c) \
             $(wildcard $(KERNEL_DIR)/drivers/*.c) \
             $(wildcard $(KERNEL_DIR)/arch/x86/*.c)
FS_SRC     = $(wildcard $(FS_DIR)/*.c)
INIT_SRC   = $(wildcard $(INIT_DIR)/*.c)
BOOT_ASM   = $(BOOT_DIR)/boot.asm

# Objetos
KERNEL_OBJ = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel_%.o,$(filter-out $(KERNEL_DIR)/arch/x86/idt.c,$(KERNEL_SRC)))
KERNEL_OBJ += $(patsubst $(KERNEL_DIR)/lib/%.c,$(BUILD_DIR)/kernel_lib_%.o,$(wildcard $(KERNEL_DIR)/lib/*.c))
KERNEL_OBJ += $(patsubst $(KERNEL_DIR)/drivers/%.c,$(BUILD_DIR)/kernel_drv_%.o,$(wildcard $(KERNEL_DIR)/drivers/*.c))
KERNEL_OBJ += $(patsubst $(KERNEL_DIR)/arch/x86/%.c,$(BUILD_DIR)/kernel_arch_%.o,$(wildcard $(KERNEL_DIR)/arch/x86/*.c))
FS_OBJ     = $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/fs_%.o,$(FS_SRC))
INIT_OBJ   = $(patsubst $(INIT_DIR)/%.c,$(BUILD_DIR)/init_%.o,$(INIT_SRC))
OBJS       = $(KERNEL_OBJ) $(FS_OBJ) $(INIT_OBJ)

# Output
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOT_BIN   = $(BUILD_DIR)/boot.bin
FLOPPY_IMG = $(OUTPUT_DIR)/floppy.img
KERNEL_SECTORS_H = $(BUILD_DIR)/kernel_sectors.inc

# --- Subdirectorios en build ---
SUBDIRS = $(BUILD_DIR) \
          $(BUILD_DIR)/kernel_lib \
          $(BUILD_DIR)/kernel_drivers \
          $(BUILD_DIR)/kernel_arch \
          $(BUILD_DIR)/fs \
          $(BUILD_DIR)/init

$(BUILD_DIR):
	mkdir -p $(SUBDIRS)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# --- Regla principal ---
all: $(FLOPPY_IMG)

# Compilar kernel
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_lib_%.o: $(KERNEL_DIR)/lib/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_drv_%.o: $(KERNEL_DIR)/drivers/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_arch_%.o: $(KERNEL_DIR)/arch/x86/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar FS
$(BUILD_DIR)/fs_%.o: $(FS_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar INIT
$(BUILD_DIR)/init_%.o: $(INIT_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Bootloader
$(BOOT_BIN): $(BOOT_ASM) $(KERNEL_SECTORS_H) | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

# Generar kernel_sectors.inc
$(KERNEL_SECTORS_H): $(KERNEL_BIN)
	actual_size=$$(stat -c %s $<); \
	mod512=$$(( $$actual_size % 512 )); \
	if [ $$mod512 -ne 0 ]; then \
		pad_size=$$((512 - $$mod512)); \
		dd if=/dev/zero bs=1 count=$$pad_size >> $< 2>/dev/null; \
		actual_size=$$(( $$actual_size + $$pad_size )); \
	fi; \
	kernel_sectors=$$(( ($$actual_size + 511) / 512 )); \
	echo "%define KERNEL_SECTORS $$kernel_sectors" > $@

# Enlazar kernel
$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# Convertir a binario plano
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Crear imagen de disquete
$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN) | $(OUTPUT_DIR)
	dd if=/dev/zero of=$@ bs=1024 count=1440 status=none
	dd if=$(BOOT_BIN) of=$@ bs=512 count=1 conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc status=none

# Ejecutar en QEMU
run: $(FLOPPY_IMG)
	qemu-system-x86_64 -drive format=raw,file=$(FLOPPY_IMG),if=floppy -boot a -m 128M -accel tcg

# Limpiar
clean:
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

.PHONY: all clean run
