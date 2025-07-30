
# --- Makefile limpio y robusto para OS x86_64 custom ---

# Herramientas
NASM = nasm
GCC = x86_64-elf-gcc
LD = x86_64-elf-ld
OBJCOPY = x86_64-elf-objcopy

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
FS_DIR = fs
INIT_DIR = init
INCLUDE_DIR = include
BUILD_DIR = build
OUTPUT_DIR = output
ISO_DIR = $(BUILD_DIR)/isodir

BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_LD = $(KERNEL_DIR)/kernel.ld
FS_C_FILES = $(wildcard $(FS_DIR)/*.c)
INIT_C_FILES = $(wildcard $(INIT_DIR)/*.c)
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)

# Archivos de salida
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_O_FILES = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel_%.o,$(KERNEL_C_FILES))
FS_O_FILES = $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/fs_%.o,$(FS_C_FILES))
INIT_O_FILES = $(patsubst $(INIT_DIR)/%.c,$(BUILD_DIR)/init_%.o,$(INIT_C_FILES))
OS_ISO = $(OUTPUT_DIR)/os.iso

# Flags
CFLAGS = -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
		 -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
		 -Wall -Wextra -Os -fdata-sections -ffunction-sections \
		 -I$(INCLUDE_DIR)
LDFLAGS = -n -nostdlib -T $(KERNEL_LD) --gc-sections
NASMFLAGS = -f bin

# Objetivo principal
all: $(OS_ISO)

# --- Reglas de construcción ---

# Boot sector (2048 bytes), incluye el header de sectores




# Boot sector + loader64 unificados (6144 bytes, 3 sectores)
$(BOOT_BIN): $(BOOT_ASM)
	@echo "[+] Compilando boot sector + loader64..."
	mkdir -p $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@
	actual_size=$$(stat -c %s $@); \
	if [ $$actual_size -lt 6144 ]; then \
		pad_size=$$((6144 - $$actual_size)); \
		dd if=/dev/zero bs=1 count=$$pad_size >> $@ 2>/dev/null; \
	fi; \
	truncate -s 6144 $@




# Kernel (binario plano, solo enlazado y convertido a binario, y genera kernel_sectors.inc)
KERNEL_SECTORS_H = $(BUILD_DIR)/kernel_sectors.inc
$(KERNEL_BIN): $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES)
	@echo "[+] Enlazando kernel..."
	mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $(BUILD_DIR)/kernel_temp.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel_temp.elf $@
	rm $(BUILD_DIR)/kernel_temp.elf
	@# Calcular sectores del kernel alineado y generar kernel_sectors.inc
	actual_size=$$(stat -c %s $@); \
	modulo=$$(($$actual_size % 2048)); \
	if [ $$modulo -ne 0 ]; then \
		pad_size=$$((2048 - $$modulo)); \
		dd if=/dev/zero bs=1 count=$$pad_size >> $@ 2>/dev/null; \
		actual_size=$$(($$actual_size + $$pad_size)); \
	fi; \
	kernel_sectors=$$(( ($$actual_size + 2047) / 2048 )); \
	if [ $$kernel_sectors -lt 1 ]; then kernel_sectors=1; fi; \
	echo "[i] kernel.bin ocupa $$kernel_sectors sectores (2048 bytes cada uno)"; \
	echo "%define KERNEL_SECTORS $$kernel_sectors" > $(KERNEL_SECTORS_H)

# Objetos de init
$(BUILD_DIR)/init_%.o: $(INIT_DIR)/%.c
	@echo "[+] Compilando init $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Objetos de kernel
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c
	@echo "[+] Compilando kernel $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Objetos de fs
$(BUILD_DIR)/fs_%.o: $(FS_DIR)/%.c
	@echo "[+] Compilando fs $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@


# --- ISO con boot.bin como sector de arranque El Torito (no emulación, sin padding extra) ---
$(OS_ISO): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "[+] Generando ISO puro (El Torito, no emulación, boot.bin directo)..."
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)
	cp $(BOOT_BIN) $(ISO_DIR)/boot.bin
	mkdir -p $(OUTPUT_DIR)
	# El estándar El Torito requiere al menos 2048 bytes, pero BIOS solo usa los primeros 512
	genisoimage -o $@ \
		-b boot.bin \
		-no-emul-boot \
		-boot-load-size 4 \
		-boot-info-table \
		-quiet \
		$(ISO_DIR)

# --- Limpieza ---
clean:
	@echo "[•] Limpiando..."
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

# --- Ejecutar en QEMU ---
run: $(OS_ISO)
	qemu-system-x86_64 -cdrom $(OS_ISO)

.PHONY: all clean run
