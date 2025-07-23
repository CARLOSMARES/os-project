# Configuración de compiladores y herramientas
NASM = nasm
GCC = x86_64-elf-gcc
LD = x86_64-elf-ld
GRUB_MKRESCUE = grub-mkrescue
QEMU = qemu-system-x86_64

# Directorios
BOOT_DIR = boot
KERNEL_DIR = kernel
FS_DIR = fs
INIT_DIR = init
GRUB_DIR = grub
INCLUDE_DIR = include
ISODIR = isodir
GRUB_ISODIR = $(ISODIR)/boot/grub

# Archivos fuente
BOOT_ASM = $(BOOT_DIR)/boot.asm
KERNEL_C_FILES = $(wildcard $(KERNEL_DIR)/*.c)
FS_C_FILES = $(wildcard $(FS_DIR)/*.c)
INIT_C_FILES = $(wildcard $(INIT_DIR)/*.c)
KERNEL_LD = $(KERNEL_DIR)/kernel.ld
GRUB_CFG = $(GRUB_DIR)/grub.cfg

# Archivos objeto y ejecutables
BOOT_O = boot.o
KERNEL_O_FILES = $(patsubst $(KERNEL_DIR)/%.c,kernel_%.o,$(KERNEL_C_FILES))
FS_O_FILES = $(patsubst $(FS_DIR)/%.c,fs_%.o,$(FS_C_FILES))
INIT_O_FILES = $(patsubst $(INIT_DIR)/%.c,init_%.o,$(INIT_C_FILES))
KERNEL_ELF = kernel.elf
OS_ISO = os.iso

# Flags del compilador para SO desde cero (sin librerías del sistema)
CFLAGS = -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -Wall -Wextra -Werror -I$(INCLUDE_DIR)
LDFLAGS = -n -nostdlib -T $(KERNEL_LD)
NASMFLAGS = -f elf64

# Objetivo principal
all: $(OS_ISO)

# Ejecutar el OS en QEMU
run: $(OS_ISO)
	@echo "[✓] Ejecutando..."
	$(QEMU) -cdrom $(OS_ISO)

# Ejecutar ISO existente sin compilar
run-iso:
	@echo "[•] Verificando si existe $(OS_ISO)..."
	@if [ -f $(OS_ISO) ]; then \
		echo "[✓] Ejecutando ISO existente..."; \
		$(QEMU) -cdrom $(OS_ISO); \
	else \
		echo "[!] Error: No se encontró $(OS_ISO)"; \
		echo "[•] Ejecute 'make all' o 'make build-clean' primero para crear el ISO"; \
		exit 1; \
	fi

# Ejecutar ISO con opciones adicionales de QEMU
run-iso-debug:
	@echo "[•] Verificando si existe $(OS_ISO)..."
	@if [ -f $(OS_ISO) ]; then \
		echo "[✓] Ejecutando ISO con debug habilitado..."; \
		$(QEMU) -cdrom $(OS_ISO) -monitor stdio -d int,cpu_reset; \
	else \
		echo "[!] Error: No se encontró $(OS_ISO)"; \
		echo "[•] Ejecute 'make all' o 'make build-clean' primero para crear el ISO"; \
		exit 1; \
	fi

# Ejecutar ISO en modo gráfico mejorado
run-iso-gui:
	@echo "[•] Verificando si existe $(OS_ISO)..."
	@if [ -f $(OS_ISO) ]; then \
		echo "[✓] Ejecutando ISO en modo gráfico..."; \
		$(QEMU) -cdrom $(OS_ISO) -vga std -display gtk; \
	else \
		echo "[!] Error: No se encontró $(OS_ISO)"; \
		echo "[•] Ejecute 'make all' o 'make build-clean' primero para crear el ISO"; \
		exit 1; \
	fi

# Crear la ISO
$(OS_ISO): $(KERNEL_ELF) prepare-grub
	@echo "[+] Creando ISO..."
	$(GRUB_MKRESCUE) -o $(OS_ISO) $(ISODIR)

# Enlazar el kernel
$(KERNEL_ELF): $(BOOT_O) $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES)
	@echo "[+] Enlazando kernel..."
	$(LD) $(LDFLAGS) $(BOOT_O) $(INIT_O_FILES) $(KERNEL_O_FILES) $(FS_O_FILES) -o $(KERNEL_ELF)

# Compilar el archivo de arranque
$(BOOT_O): $(BOOT_ASM)
	@echo "[+] Compilando boot.asm..."
	$(NASM) $(NASMFLAGS) $(BOOT_ASM) -o $(BOOT_O)

# Compilar init (sistema de inicialización)
$(INIT_O_FILES): init_%.o: $(INIT_DIR)/%.c
	@echo "[+] Compilando sistema de inicialización $<..."
	$(GCC) $(CFLAGS) -c $< -o $@

# Compilar el kernel
$(KERNEL_O_FILES): kernel_%.o: $(KERNEL_DIR)/%.c
	@echo "[+] Compilando $<..."
	$(GCC) $(CFLAGS) -c $< -o $@

# Compilar el sistema de archivos
$(FS_O_FILES): fs_%.o: $(FS_DIR)/%.c
	@echo "[+] Compilando sistema de archivos $<..."
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

# Limpiar solo archivos de compilación, conservar ISO
clean-build:
	@echo "[•] Limpiando archivos de compilación (conservando ISO)..."
	rm -rf $(ISODIR)
	rm -f *.o *.elf *.qcow2

# Construir y limpiar - Solo deja el archivo ISO
build-clean: $(OS_ISO)
	@echo "[•] Limpiando archivos temporales (conservando $(OS_ISO))..."
	rm -rf $(ISODIR)
	rm -f *.o *.elf *.qcow2
	@echo "[✓] Compilación completa. Solo queda: $(OS_ISO)"
	@ls -lah $(OS_ISO)

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
.PHONY: help
help:
	@echo "================================================"
	@echo "           MicroCIOMOS - Comandos Make"
	@echo "================================================"
	@echo "Comandos principales:"
	@echo "  make all         - Compilar todo el sistema"
	@echo "  make run         - Compilar y ejecutar en QEMU"
	@echo "  make build-clean - Compilar y limpiar (solo deja ISO)"
	@echo ""
	@echo "Comandos de ejecución directa:"
	@echo "  make run-iso     - Ejecutar ISO existente (sin compilar)"
	@echo "  make run-iso-debug - Ejecutar ISO con debug habilitado"
	@echo "  make run-iso-gui   - Ejecutar ISO en modo gráfico"
	@echo ""
	@echo "Comandos de limpieza:"
	@echo "  make clean       - Limpiar todo (incluye ISO)"
	@echo "  make clean-build - Limpiar compilación (conserva ISO)"
	@echo "  make clean-all   - Limpieza completa + respaldos"
	@echo ""
	@echo "Otros comandos:"
	@echo "  make create-hdd-image - Crear imagen de disco virtual"
	@echo "  make install-to-hdd   - Instalar en disco virtual"
	@echo "  make run-hdd         - Ejecutar desde disco virtual"
	@echo "  make create-usb      - Mostrar info para USB booteable"
	@echo "  make install-usb     - Crear USB booteable"
	@echo ""
	@echo "Archivos generados:"
	@echo "  kernel.elf       - Ejecutable del kernel"
	@echo "  os.iso          - Imagen ISO booteable"
	@echo "================================================"

# Directivas PHONY para asegurar que estos comandos siempre se ejecuten
.PHONY: all run run-iso run-iso-debug run-iso-gui clean clean-build clean-all help prepare-grub build-clean create-hdd-image install-to-hdd run-hdd install-to-physical install-to-disk create-usb install-usb
