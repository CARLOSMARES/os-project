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
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_LD = $(KERNEL_DIR)/kernel.ld
GRUB_CFG = $(GRUB_DIR)/grub.cfg

# Archivos objeto y ejecutables
BOOT_O = boot.o
KERNEL_O_FILES = $(patsubst $(KERNEL_DIR)/%.c,%.o,$(KERNEL_C_FILES))
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
$(KERNEL_ELF): $(BOOT_O) $(KERNEL_O_FILES)
	@echo "[+] Enlazando kernel..."
	$(LD) $(LDFLAGS) $(BOOT_O) $(KERNEL_O_FILES) -o $(KERNEL_ELF)

# Compilar el archivo de arranque
$(BOOT_O): $(BOOT_ASM)
	@echo "[+] Compilando boot.asm..."
	$(NASM) $(NASMFLAGS) $(BOOT_ASM) -o $(BOOT_O)

# Compilar el kernel
$(KERNEL_O_FILES): %.o: $(KERNEL_DIR)/%.c
	@echo "[+] Compilando $<..."
	$(GCC) $(CFLAGS) -c $< -o $@

# Preparar el directorio GRUB
prepare-grub: $(KERNEL_ELF)
	@echo "[+] Preparando GRUB..."
	mkdir -p $(GRUB_ISODIR)
	mkdir -p $(ISODIR)/usr/include
	cp $(KERNEL_ELF) $(ISODIR)/boot/$(KERNEL_ELF)
	cp $(GRUB_CFG) $(GRUB_ISODIR)/grub.cfg
	cp -r $(INCLUDE_DIR)/* $(ISODIR)/usr/include/
	@echo "[•] Verificando archivos..."
	ls -la $(ISODIR)/boot/
	ls -la $(GRUB_ISODIR)/
	@echo "[•] Headers incluidos en /usr/include:"
	ls -la $(ISODIR)/usr/include/

# Limpiar archivos generados
clean:
	@echo "[•] Limpiando..."
	rm -rf $(ISODIR)
	rm -f *.o *.bin *.iso *.elf *.qcow2

# Limpiar completamente (incluyendo archivos de respaldo)
clean-all: clean
	@echo "[•] Limpieza completa..."
	rm -f *~ $(BOOT_DIR)/*~ $(KERNEL_DIR)/*~ $(GRUB_DIR)/*~ $(INCLUDE_DIR)/*~

# Crear imagen de disco duro virtual para pruebas
create-hdd-image:
	@echo "[+] Creando imagen de disco duro virtual..."
	qemu-img create -f qcow2 os-hdd.qcow2 1G
	@echo "[✓] Imagen de disco creada: os-hdd.qcow2"

# Particionar e instalar en imagen de disco virtual
install-to-hdd: $(OS_ISO) create-hdd-image
	@echo "[+] Instalando SO en disco duro virtual..."
	@echo "[!] Esto instalará GRUB y el kernel en la imagen de disco"
	@echo "[•] Iniciando instalación automática..."
	sudo mkdir -p /mnt/os-install
	sudo losetup -D
	sudo qemu-nbd --connect=/dev/nbd0 os-hdd.qcow2 || true
	sleep 2
	@echo "[•] Particionando disco..."
	sudo parted /dev/nbd0 mklabel msdos
	sudo parted /dev/nbd0 mkpart primary ext4 1MiB 100%
	sudo parted /dev/nbd0 set 1 boot on
	sleep 2
	@echo "[•] Formateando partición..."
	sudo mkfs.ext4 /dev/nbd0p1
	@echo "[•] Montando partición..."
	sudo mount /dev/nbd0p1 /mnt/os-install
	@echo "[•] Copiando archivos del sistema..."
	sudo mkdir -p /mnt/os-install/boot/grub
	sudo mkdir -p /mnt/os-install/usr/include
	sudo cp $(KERNEL_ELF) /mnt/os-install/boot/
	sudo cp $(GRUB_CFG) /mnt/os-install/boot/grub/
	sudo cp -r $(INCLUDE_DIR)/* /mnt/os-install/usr/include/
	@echo "[•] Instalando GRUB..."
	sudo grub-install --target=i386-pc --boot-directory=/mnt/os-install/boot /dev/nbd0
	@echo "[•] Desmontando..."
	sudo umount /mnt/os-install
	sudo qemu-nbd --disconnect /dev/nbd0
	sudo rmdir /mnt/os-install
	@echo "[✓] Instalación completada en os-hdd.qcow2"

# Ejecutar desde disco duro virtual
run-hdd: install-to-hdd
	@echo "[✓] Ejecutando desde disco duro virtual..."
	$(QEMU) -hda os-hdd.qcow2

# Instalar en disco duro físico (¡PELIGROSO!)
install-to-physical:
	@echo "[!] ==================== ADVERTENCIA ===================="
	@echo "[!] Esto BORRARÁ COMPLETAMENTE el disco especificado"
	@echo "[!] Asegúrate de hacer backup de todos tus datos importantes"
	@echo "[!] ======================================================="
	@echo ""
	@echo "Discos disponibles:"
	@lsblk -d -o NAME,SIZE,TYPE,MODEL
	@echo ""
	@echo "Para continuar, ejecuta:"
	@echo "  sudo make install-to-disk DISK=/dev/sdX"
	@echo "  (reemplaza sdX con tu disco, ej: sdb, sdc, etc.)"
	@echo ""
	@echo "[!] NO uses tu disco principal (normalmente sda)"

# Instalación real en disco físico
install-to-disk: $(OS_ISO)
ifndef DISK
	@echo "[!] Error: Debes especificar DISK=/dev/sdX"
	@echo "Ejemplo: sudo make install-to-disk DISK=/dev/sdb"
	@exit 1
endif
	@echo "[!] ==================== ÚLTIMA ADVERTENCIA ===================="
	@echo "[!] Vas a instalar en: $(DISK)"
	@echo "[!] Esto BORRARÁ COMPLETAMENTE todos los datos en $(DISK)"
	@echo "[!] ==============================================================="
	@echo ""
	@echo "Información del disco:"
	@lsblk $(DISK)
	@echo ""
	@read -p "¿Estás ABSOLUTAMENTE seguro? Escribe 'CONFIRMAR': " confirm && [ "$$confirm" = "CONFIRMAR" ]
	@echo "[•] Desmontando particiones existentes..."
	-sudo umount $(DISK)* 2>/dev/null || true
	@echo "[•] Particionando $(DISK)..."
	sudo parted $(DISK) mklabel msdos
	sudo parted $(DISK) mkpart primary ext4 1MiB 100%
	sudo parted $(DISK) set 1 boot on
	@echo "[•] Formateando partición..."
	sudo mkfs.ext4 $(DISK)1
	@echo "[•] Montando partición..."
	sudo mkdir -p /mnt/os-install
	sudo mount $(DISK)1 /mnt/os-install
	@echo "[•] Copiando archivos del sistema..."
	sudo mkdir -p /mnt/os-install/boot/grub
	sudo mkdir -p /mnt/os-install/usr/include
	sudo cp $(KERNEL_ELF) /mnt/os-install/boot/
	sudo cp $(GRUB_CFG) /mnt/os-install/boot/grub/
	sudo cp -r $(INCLUDE_DIR)/* /mnt/os-install/usr/include/
	@echo "[•] Instalando GRUB en $(DISK)..."
	sudo grub-install --target=i386-pc --boot-directory=/mnt/os-install/boot $(DISK)
	@echo "[•] Desmontando..."
	sudo umount /mnt/os-install
	sudo rmdir /mnt/os-install
	@echo "[✓] ¡$(shell basename $(DISK)) instalado exitosamente en $(DISK)!"
	@echo "[✓] Puedes reiniciar y arrancar desde $(DISK)"

# Crear USB booteable
create-usb:
	@echo "[!] Creando USB booteable..."
	@echo "Dispositivos USB disponibles:"
	@lsblk -d -o NAME,SIZE,TYPE,TRAN | grep usb
	@echo ""
	@echo "Para crear USB booteable, ejecuta:"
	@echo "  sudo make install-usb USB=/dev/sdX"
	@echo "  (reemplaza sdX con tu USB, ej: sdb, sdc, etc.)"

# Instalar en USB
install-usb: $(OS_ISO)
ifndef USB
	@echo "[!] Error: Debes especificar USB=/dev/sdX"
	@echo "Ejemplo: sudo make install-usb USB=/dev/sdb"
	@exit 1
endif
	@echo "[!] Esto borrará todos los datos en $(USB)"
	@echo "Información del USB:"
	@lsblk $(USB)
	@read -p "¿Continuar? [y/N]: " confirm && [ "$$confirm" = "y" ]
	@echo "[•] Creando USB booteable..."
	sudo dd if=$(OS_ISO) of=$(USB) bs=4M status=progress oflag=sync
	@echo "[✓] USB booteable creado exitosamente"

# Mostrar ayuda
help:
	@echo "Makefile para el proyecto OS"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all                  - Compilar y crear la ISO (objetivo por defecto)"
	@echo "  run                  - Compilar, crear ISO y ejecutar en QEMU"
	@echo "  create-hdd-image     - Crear imagen de disco duro virtual"
	@echo "  install-to-hdd       - Instalar en disco duro virtual"
	@echo "  run-hdd             - Ejecutar desde disco duro virtual"
	@echo "  install-to-physical  - Mostrar info para instalar en disco físico"
	@echo "  install-to-disk     - Instalar en disco físico (¡PELIGROSO!)"
	@echo "  create-usb          - Mostrar info para crear USB booteable"
	@echo "  install-usb         - Crear USB booteable"
	@echo "  clean               - Limpiar archivos generados"
	@echo "  clean-all           - Limpieza completa (incluye archivos de respaldo)"
	@echo "  help                - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos de uso:"
	@echo "  make                                    - Compilar el proyecto"
	@echo "  make run                               - Compilar y ejecutar en QEMU"
	@echo "  make run-hdd                           - Instalar y ejecutar desde HDD virtual"
	@echo "  sudo make install-to-disk DISK=/dev/sdb - Instalar en disco físico"
	@echo "  sudo make install-usb USB=/dev/sdb      - Crear USB booteable"
	@echo ""
	@echo "⚠️  ADVERTENCIA: Los comandos de instalación en disco físico/USB"
	@echo "   borrarán COMPLETAMENTE el dispositivo especificado."

.PHONY: all run clean clean-all help prepare-grub create-hdd-image install-to-hdd run-hdd install-to-physical install-to-disk create-usb install-usb
