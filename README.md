# MicroCIOMOS x64 🚀

Un sistema operativo de 64 bits desarrollado desde cero, diseñado para ser simple, educativo y funcional.

## 📋 Características

- **Arquitectura:** x64 (64 bits)
- **Bootloader:** GRUB2
- **Lenguaje:** C y Assembly
- **Compilación:** Cross-compiler (x86_64-elf-gcc)
- **Emulación:** QEMU

## 🛠️ Requisitos del Sistema

### Herramientas Necesarias

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install nasm x86_64-elf-gcc x86_64-elf-ld grub-pc-bin grub-common \
                 xorriso qemu-system-x86 qemu-utils parted qemu-block-extra
```

**Arch Linux:**

```bash
sudo pacman -S nasm gcc grub qemu-base parted xorriso
```

**Fedora/RHEL:**

```bash
sudo dnf install nasm gcc grub2-tools qemu-system-x86 qemu-img parted xorriso
```

## 🚀 Compilación y Ejecución

### Compilación Básica

```bash
# Compilar el proyecto
make

# Compilar y ejecutar en QEMU
make run

# Limpiar archivos generados
make clean
```

### Instalación y Testing

```bash
# Probar instalación en disco virtual (seguro)
make run-hdd

# Crear USB booteable
make create-usb
sudo make install-usb USB=/dev/sdX  # ⚠️ Reemplaza sdX con tu USB

# Instalar en disco físico (¡PELIGROSO!)
make install-to-physical
sudo make install-to-disk DISK=/dev/sdX  # ⚠️ Solo para discos dedicados
```

### Ver Todas las Opciones

```bash
make help
```

## 📁 Estructura del Proyecto

```
os-project/
├── boot/           # Código de arranque en Assembly
│   └── boot.asm    # Bootstrap inicial
├── kernel/         # Núcleo del sistema operativo
│   ├── kernel.c    # Función principal del kernel
│   ├── stack.c     # Implementación de stack
│   └── kernel.ld   # Script de enlazado
├── include/        # Headers del sistema
│   ├── stdint.h    # Tipos de datos estándar
│   ├── types.h     # Definiciones de tipos
│   └── core/       # Headers del núcleo
│       └── stack.h # Definiciones de stack
├── grub/           # Configuración de GRUB
│   └── grub.cfg    # Menu de arranque
├── Makefile        # Sistema de compilación
├── README.md       # Este archivo
└── .gitignore      # Archivos ignorados por git
```

## ⚙️ Arquitectura

### Componentes Principales

- **Boot Loader:** GRUB2 maneja el arranque inicial
- **Kernel:** Núcleo principal escrito en C
- **Stack:** Estructura de datos implementada para manejo de memoria
- **Headers:** Sistema de inclusión modular siguiendo convenciones Unix

### Características del Build System

- ✅ **Compilación automática** de todos los archivos `.c` en `kernel/`
- ✅ **Headers incluidos** en la ISO bajo `/usr/include`
- ✅ **Múltiples targets** de instalación (virtual, USB, disco)
- ✅ **Verificaciones de seguridad** para prevenir pérdida de datos
- ✅ **Soporte para desarrollo modular**

## 🖥️ Modos de Ejecución

### 1. Desarrollo (QEMU)

```bash
make run
```

- Perfecto para desarrollo y testing
- Entorno controlado y seguro
- Debugging integrado

### 2. Testing en Hardware Virtual

```bash
make run-hdd
```

- Simula instalación en disco real
- Prueba el bootloader completo
- 100% seguro para el sistema host

### 3. Hardware Real

```bash
# USB booteable (recomendado)
sudo make install-usb USB=/dev/sdX

# Instalación permanente (solo discos dedicados)
sudo make install-to-disk DISK=/dev/sdX
```

## 🛡️ Medidas de Seguridad

- **Confirmaciones múltiples** antes de escribir en hardware
- **Listado automático** de dispositivos disponibles
- **Modo de prueba virtual** completamente aislado
- **Verificaciones de parámetros** para prevenir accidentes
- **Desmontaje automático** para prevenir corrupción de datos

## 🎯 Objetivos Educativos

Este proyecto está diseñado para enseñar:

- Desarrollo de sistemas operativos desde cero
- Programación en C de bajo nivel
- Assembly x86-64
- Bootloaders y proceso de arranque
- Gestión de memoria y estructuras de datos
- Herramientas de desarrollo de sistemas

## 📚 Próximos Desarrollos

- [ ] Sistema de gestión de memoria
- [ ] Drivers básicos (teclado, pantalla)
- [ ] Sistema de archivos simple
- [ ] Multitasking básico
- [ ] Interfaz de usuario mínima

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## ⚠️ Advertencias Importantes

- **Instalación en disco físico:** Los comandos de instalación en hardware real borrarán COMPLETAMENTE el dispositivo especificado
- **Backup obligatorio:** Siempre haz backup de datos importantes antes de usar hardware real
- **Testing recomendado:** Usa `make run-hdd` antes de probar en hardware físico

## 📄 Licencia

Este proyecto está bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para detalles.

## 👨‍💻 Autor

**Carlos Mares** - Desarrollo inicial

## 🎖️ Reconocimientos

- Comunidad de desarrollo de OS
- Documentación de GRUB2 y QEMU
- Tutoriales de OS development de OSDev.org

---

⭐ **¡Dale una estrella al proyecto si te fue útil!** ⭐
