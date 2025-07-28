# Configuración de compiladores y herramientas
NASM = nasm
GCC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
FS_DIR = fs
INIT_DIR = init
INCLUDE_DIR = include
BUILD_DIR = build
OUTPUT_DIR = output
ISODIR = $(BUILD_DIR)/isodir

# Archivos fuente
BOOT_ASM = $(BOOT_DIR)/boot.asm
BOOT_LD = $(BOOT_DIR)/boot.ld
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
FS_C_FILES = $(wildcard $(FS_DIR)/*.c)
INIT_C_FILES = $(wildcard $(INIT_DIR)/*.c)
KERNEL_LD = $(KERNEL_DIR)/kernel.ld

# Archivos objeto y ejecutables
BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_O_FILES = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel_%.o,$(KERNEL_C_FILES))
FS_O_FILES = $(patsubst $(FS_DIR)/%.c,$(BUILD_DIR)/fs_%.o,$(FS_C_FILES))
INIT_O_FILES = $(patsubst $(INIT_DIR)/%.c,$(BUILD_DIR)/init_%.o,$(INIT_C_FILES))
OS_ISO = $(OUTPUT_DIR)/os.iso

# Flags del compilador para SO desde cero (sin librerías del sistema)
CFLAGS = -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -Wall -Wextra -Werror -I$(INCLUDE_DIR)
LDFLAGS = -n -nostdlib -T $(KERNEL_LD)
NASMFLAGS = -f bin

# Objetivo principal
all: $(OS_ISO)

# Limpiar archivos generados
clean:
	@echo "[•] Limpiando..."
	rm -rf $(BUILD_DIR) $(OUTPUT_DIR)

# Crear ISO booteable con bootloader personalizado (sin GRUB)
$(OS_ISO): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "[+] Creando ISO booteable con bootloader personalizado..."
	# Crear directorios de salida
	mkdir -p $(OUTPUT_DIR)
	mkdir -p $(ISODIR)/boot
	# Crear imagen de floppy virtual que contiene bootloader + kernel
	dd if=/dev/zero of=$(ISODIR)/boot/bootdisk.img bs=512 count=2880
	dd if=$(BOOT_BIN) of=$(ISODIR)/boot/bootdisk.img bs=512 count=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$(ISODIR)/boot/bootdisk.img bs=512 seek=1 conv=notrunc
	# Crear ISO con El Torito usando la imagen de floppy como boot image
	genisoimage -quiet -V "MicroCIOMOS" -input-charset iso8859-1 \
		-o $(OS_ISO) -b boot/bootdisk.img -hide boot/bootdisk.img \
		-boot-load-size 4 -boot-info-table -no-emul-boot $(ISODIR)
	@echo "[✓] ISO creada: $(OS_ISO)"

# Compilar el bootloader
$(BOOT_BIN): $(BOOT_ASM)
	@echo "[+] Compilando bootloader..."
	mkdir -p $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $(BOOT_ASM) -o $(BOOT_BIN)

# Compilar el kernel
$(KERNEL_BIN): $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES)
	@echo "[+] Enlazando kernel..."
	mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES) -o $(BUILD_DIR)/kernel_temp.elf
	objcopy -O binary $(BUILD_DIR)/kernel_temp.elf $(KERNEL_BIN)
	rm $(BUILD_DIR)/kernel_temp.elf

# Compilar init (sistema de inicialización)
$(BUILD_DIR)/init_%.o: $(INIT_DIR)/%.c
	@echo "[+] Compilando sistema de inicialización $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Compilar el kernel
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c
	@echo "[+] Compilando $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Compilar el sistema de archivos
$(BUILD_DIR)/fs_%.o: $(FS_DIR)/%.c
	@echo "[+] Compilando sistema de archivos $<..."
	mkdir -p $(BUILD_DIR)
	$(GCC) $(CFLAGS) -c $< -o $@

# Directivas PHONY para asegurar que estos comandos siempre se ejecuten
.PHONY: all clean
