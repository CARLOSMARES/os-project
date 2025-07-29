
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

# Archivos fuente
BOOT_ASM = $(BOOT_DIR)/boot.asm
LOADER64_ASM = $(BOOT_DIR)/loader64.asm
KERNEL_LD = $(KERNEL_DIR)/kernel.ld
FS_C_FILES = $(wildcard $(FS_DIR)/*.c)
INIT_C_FILES = $(wildcard $(INIT_DIR)/*.c)
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)

# Archivos de salida
BOOT_BIN = $(BUILD_DIR)/boot.bin
LOADER64_BIN = $(BUILD_DIR)/loader64.bin
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

# Boot sector (2048 bytes)
$(BOOT_BIN): $(BOOT_ASM)
	@echo "[+] Compilando boot sector..."
	mkdir -p $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@

# Loader64 (4096 bytes)
$(LOADER64_BIN): $(LOADER64_ASM)
	@echo "[+] Ensamblando loader64..."
	mkdir -p $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@
	truncate -s 4096 $@

# Kernel (binario plano)
$(KERNEL_BIN): $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES)
	@echo "[+] Enlazando kernel..."
	mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $^ -o $(BUILD_DIR)/kernel_temp.elf
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel_temp.elf $@
	rm $(BUILD_DIR)/kernel_temp.elf

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

# --- ISO puro con loader64 en sector 17 (El Torito, no emulación) ---
$(OS_ISO): $(BOOT_BIN) $(LOADER64_BIN) $(KERNEL_BIN)
	@echo "[+] Generando ISO puro (El Torito, no emulación)..."
	rm -rf $(ISO_DIR)
	mkdir -p $(ISO_DIR)
	cp $< $(ISO_DIR)/boot.bin
	dd if=/dev/zero of=$(ISO_DIR)/pad bs=2048 count=15 status=none
	cat $(ISO_DIR)/boot.bin $(ISO_DIR)/pad $(LOADER64_BIN) > $(ISO_DIR)/bootfull.img
	mkdir -p $(OUTPUT_DIR)
	genisoimage -o $@ \
		-b bootfull.img \
		-no-emul-boot \
		-boot-load-size 18 \
		-boot-info-table \
		-quiet \
		$(ISO_DIR)
	rm $(ISO_DIR)/pad $(ISO_DIR)/bootfull.img $(ISO_DIR)/boot.bin

# --- Limpieza ---
clean:
	@echo "[•] Limpiando..."
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

.PHONY: all clean
