# --- Makefile para OS x86 32-bit ---

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

# Flags
CFLAGS  = -ffreestanding -O2 -Wall -Wextra -fno-stack-protector -Iinclude $(CFLAGS_ARCH)
LDFLAGS = -n -nostdlib -T kernel/kernel.ld --gc-sections $(LDFLAGS_ARCH)

# Directorios
BUILD_DIR  = build
OUTPUT_DIR = output

# Subdirectorios de build
SUBDIRS = $(BUILD_DIR) \
          $(BUILD_DIR)/kernel \
          $(BUILD_DIR)/fs \
          $(BUILD_DIR)/init

# Fuentes
KERNEL_SRC := $(shell find kernel -name '*.c')
FS_SRC     := $(shell find fs -name '*.c')
INIT_SRC   := $(shell find init -name '*.c')
BOOT_ASM   := boot/boot.asm
IDT_ASM    := kernel/arch/x86/idt_stubs.s

# Objetos
KERNEL_OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_SRC))
FS_OBJ     := $(patsubst %.c,$(BUILD_DIR)/%.o,$(FS_SRC))
INIT_OBJ   := $(patsubst %.c,$(BUILD_DIR)/%.o,$(INIT_SRC))
STUBS_OBJ  := $(BUILD_DIR)/idt_stubs.o

OBJS       := $(KERNEL_OBJ) $(FS_OBJ) $(INIT_OBJ) $(STUBS_OBJ)

# Output
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
BOOT_BIN   = $(BUILD_DIR)/boot.bin
FLOPPY_IMG = $(OUTPUT_DIR)/floppy.img
KERNEL_SECTORS_H = $(BUILD_DIR)/kernel_sectors.inc

# Crear subdirectorios
$(BUILD_DIR):
	mkdir -p $(SUBDIRS)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

# Regla principal
all: $(FLOPPY_IMG)

# Compilar C a objetos
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar stubs ASM
$(STUBS_OBJ): $(IDT_ASM) | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

# Compilar bootloader
$(BOOT_BIN): $(BOOT_ASM) $(KERNEL_SECTORS_H) | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

# Enlazar kernel
$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# Convertir a binario plano
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Generar kernel_sectors.inc para el bootloader
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
