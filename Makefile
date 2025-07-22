# Configuración de compiladores y herramientas
NASM = nasm
GCC = x86_64-elf-gcc
LD = x86_64-elf-ld
GRUB_MKRESCUE = grub-mkrescue
QEMU = qemu-system-x86_64

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
GRUB_DIR = grub
INCLUDE_DIR = include
ISODIR = isodir
GRUB_ISODIR = $(ISODIR)/boot/grub

# Archivos fuente
BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_C = $(KERNEL_DIR)/kernel.c
KERNEL_LD = $(KERNEL_DIR)/kernel.ld
GRUB_CFG = $(GRUB_DIR)/grub.cfg

# Archivos objeto y ejecutables
BOOT_O = boot.o
KERNEL_O = kernel.o
KERNEL_ELF = kernel.elf
OS_ISO = os.iso

# Flags del compilador
CFLAGS = -ffreestanding -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -I$(INCLUDE_DIR)
LDFLAGS = -n -T $(KERNEL_LD)
NASMFLAGS = -f elf64

# Objetivo principal
all: $(OS_ISO)

# Ejecutar el OS en QEMU
run: $(OS_ISO)
	@echo "[✓] Ejecutando..."
	$(QEMU) -cdrom $(OS_ISO)

# Crear la ISO
$(OS_ISO): $(KERNEL_ELF) prepare-grub
	@echo "[+] Creando ISO..."
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISODIR)

# Enlazar el kernel
$(KERNEL_ELF): $(BOOT_O) $(KERNEL_O)
	@echo "[+] Enlazando kernel..."
	$(LD) $(LDFLAGS) $(BOOT_O) $(KERNEL_O) -o $(KERNEL_ELF)

# Compilar el archivo de arranque
$(BOOT_O): $(BOOT_ASM)
	@echo "[+] Compilando boot.asm..."
	$(NASM) $(NASMFLAGS) $(BOOT_ASM) -o $(BOOT_O)

# Compilar el kernel
$(KERNEL_O): $(KERNEL_C)
	@echo "[+] Compilando kernel.c..."
	$(GCC) $(CFLAGS) -c $(KERNEL_C) -o $(KERNEL_O)

# Preparar el directorio GRUB
prepare-grub: $(KERNEL_ELF)
	@echo "[+] Preparando GRUB..."
	mkdir -p $(GRUB_ISODIR)
	cp $(KERNEL_ELF) $(ISODIR)/boot/$(KERNEL_ELF)
	cp $(GRUB_CFG) $(GRUB_ISODIR)/grub.cfg
	@echo "[•] Verificando archivos..."
	ls -la $(ISODIR)/boot/
	ls -la $(GRUB_ISODIR)/

# Limpiar archivos generados
clean:
	@echo "[•] Limpiando..."
	rm -rf $(ISODIR)
	rm -f *.o *.bin *.iso *.elf

# Limpiar completamente (incluyendo archivos de respaldo)
clean-all: clean
	@echo "[•] Limpieza completa..."
	rm -f *~ $(BOOT_DIR)/*~ $(KERNEL_DIR)/*~ $(GRUB_DIR)/*~ $(INCLUDE_DIR)/*~

# Mostrar ayuda
help:
	@echo "Makefile para el proyecto OS"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all         - Compilar y crear la ISO (objetivo por defecto)"
	@echo "  run         - Compilar, crear ISO y ejecutar en QEMU"
	@echo "  clean       - Limpiar archivos generados"
	@echo "  clean-all   - Limpieza completa (incluye archivos de respaldo)"
	@echo "  help        - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos:"
	@echo "  make        - Compilar el proyecto"
	@echo "  make run    - Compilar y ejecutar"
	@echo "  make clean  - Limpiar archivos generados"

.PHONY: all run clean clean-all help prepare-grub
