# MicroCIOMOS - Sistema Operativo Educativo 🚀

Sistema operativo básico para aprendizaje, desarrollado desde cero para arquitectura x86_64.

## 🛠️ Requisitos

- nasm
- x86_64-elf-gcc, x86_64-elf-ld y x86_64-elf-objcopy (toolchain cruzado)
- genisoimage (o equivalente que provea `genisoimage`)
- qemu-system-x86_64

Ejemplos de instalación (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install nasm qemu-system-x86 genisoimage
# Necesitas además un toolchain cruzado x86_64-elf (ver guía de OSDev)
```

## 🚀 Compilar y Ejecutar

### 1) Compilar ISO

```bash
make          # o: make all
```

Salida: `output/os.iso`

### 2) Ejecutar en QEMU

```bash
make run
```

Esto abrirá QEMU usando el ISO generado.

### 3) Limpiar artefactos

```bash
make clean
```

## ▶️ Script de ejecución opcional

También puedes usar el script incluido para lanzar QEMU sobre el ISO ya generado:

```bash
bash test_iso.sh
```

## 📁 Estructura mínima

```
os-project/
├── boot/        # Código de arranque (ASM)
├── init/        # Inicialización del sistema (C)
├── kernel/      # Núcleo (C) y linker script
├── fs/          # Componentes del sistema de archivos (C)
├── include/     # Headers
├── Makefile     # Build del ISO y ejecución en QEMU
├── test_iso.sh  # Script simple para ejecutar el ISO
└── README.md
```

## ℹ️ Notas

- El `Makefile` espera un toolchain cruzado prefijado como `x86_64-elf-*`.
- Si no cuentas con él, sigue una guía de instalación del toolchain (OSDev Wiki) o usa un contenedor con dichas herramientas.
