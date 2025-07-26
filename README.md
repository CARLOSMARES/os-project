# MicroCIOMOS - Sistema Operativo Educativo 🚀

Sistema operativo básico para aprendizaje, desarrollado desde cero con arquitectura x86_64 y sistema de archivos completo.

## 📋 Características

- **Kernel modular** en C con inicialización multicapa
- **Bootloader personalizado** con NASM
- **Sistema de archivos completo** con inodos y directorios
- **Arquitectura de arranque**: boot → init → kernel
- **Salida VGA** con colores y formateo avanzado
- **Scripts de ejecución rápida** para desarrollo ágil

## 🛠️ Requisitos del Sistema

### Herramientas Necesarias

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install nasm gcc ld grub-pc-bin grub-common \
                 xorriso qemu-system-x86 qemu-utils
```

**Arch Linux:**

```bash
sudo pacman -S nasm gcc grub qemu-base xorriso
```

**Fedora/RHEL:**

```bash
sudo dnf install nasm gcc grub2-tools qemu-system-x86 xorriso
```

## 🚀 Compilación y Ejecución

### ⚡ Ejecución Rápida (Recomendado)

```bash
# Compilar una vez y limpiar
make build-clean

# Ejecutar ISO existente (sin recompilar)
make run-iso

# Ejecutar con debug habilitado
make run-iso-debug

# Ejecutar en modo gráfico mejorado
make run-iso-gui
```

### 🎯 Script Independiente

```bash
# Hacer ejecutable (primera vez)
chmod +x run.sh

# Ejecutar en modo normal
./run.sh

# Con opciones específicas
./run.sh --debug   # Debug habilitado
./run.sh --gui     # Modo gráfico
./run.sh --info    # Información del ISO
./run.sh --help    # Ayuda completa
```

### 🔄 Compilación Tradicional

```bash
# Compilar todo
make all

# Compilar y ejecutar
make run

# Limpiar archivos
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

os-project/
├── boot/           # Código de arranque en Assembly
│   └── boot.asm    # Bootstrap inicial que llama a main()
├── init/           # Inicialización del sistema
│   └── main.c      # Punto de entrada principal del sistema
├── kernel/         # Núcleo del sistema operativo
│   ├── kernel.c    # Servicios principales del kernel
│   ├── stack.c     # Implementación de stack
│   ├── stdio.c     # E/S estándar
│   ├── vga_color.c # Manejo de colores VGA
│   ├── panic.c     # Manejo de errores críticos
│   └── kernel.ld   # Script de enlazado
├── fs/             # Sistema de archivos completo
│   ├── fs.c        # Implementación core con inodos
│   └── file.c      # Operaciones de archivos (POSIX-like)
├── include/        # Headers del sistema
│   ├── stdint.h    # Tipos de datos estándar
│   ├── sys         # Headers del sys
│       └──types.h     # Definiciones de tipos
│   ├── stdio.h     # E/O estándar
│   ├── fs.h        # Sistema de archivos
│   ├── file.h      # Operaciones de archivos
│   └── core/       # Headers del núcleo
│       └── stack.h # Definiciones de stack
├── grub/           # Configuración de GRUB
│   └── grub.cfg    # Menu de arranque
├── Makefile        # Sistema de compilación avanzado
├── run.sh          # Script de ejecución independiente
├── README.md       # Documentación principal
├── EJECUCION_ISO.md # Guía de comandos de ejecución
└── .gitignore      # Archivos ignorados por git

## ⚙️ Arquitectura del Sistema

### 🚀 Secuencia de Arranque

1. **GRUB** carga el bootloader
2. **boot.asm** configura el entorno inicial
3. **init/main.c** inicializa hardware y servicios
4. **kernel/kernel.c** ejecuta servicios principales

### 🗂️ Sistema de Archivos

- **Superbloque** con metadatos del filesystem
- **Bitmap de inodos** para gestión de archivos
- **Bitmap de bloques** para gestión de espacio
- **Directorios** con soporte jerárquico
- **API POSIX-like** (open, read, write, seek, close)

### 🔧 Características del Build System

- ✅ **Compilación automática** con dependencias
- ✅ **Limpieza selectiva** (build-clean mantiene solo ISO)
- ✅ **Múltiples modos de ejecución** (normal, debug, GUI)
- ✅ **Script independiente** para uso conveniente
- ✅ **Verificaciones de integridad** del ISO

## 🖥️ Modos de Ejecución

### 1. Desarrollo Rápido (⚡ Recomendado)

```bash
make run-iso      # Ejecución rápida sin recompilar
./run.sh          # Script independiente
```

### 2. Debug y Diagnóstico

```bash
make run-iso-debug    # Debug con monitor QEMU
./run.sh --debug      # Script con debug habilitado
```

### 3. Presentaciones y Demos

```bash
make run-iso-gui      # Interfaz gráfica mejorada
./run.sh --gui        # Script con modo gráfico
```

### 4. Compilación Completa

```bash
make run              # Compilar y ejecutar
```

## 📖 Guías de Uso

### 🎯 Para Desarrollo Activo

```bash
# 1. Compilar una vez
make build-clean

# 2. Pruebas rápidas repetidas
make run-iso

# 3. Debug cuando sea necesario
make run-iso-debug
```

### 🚀 Para Demos

```bash
# Verificar estado del ISO
./run.sh --info

# Ejecutar en modo gráfico
./run.sh --gui
```

### 🔍 Para Diagnóstico

```bash
# Ver toda la ayuda
make help
./run.sh --help

# Ejecutar con debug completo
make run-iso-debug
```

## 🎓 Objetivos Educativos

- 🧠 **Entender el arranque completo** desde bootloader hasta kernel
- 🗂️ **Implementar filesystem** con inodos y gestión de bloques
- 🔧 **Arquitectura modular** con separación de responsabilidades
- 💾 **Programación de sistemas** con manejo directo de hardware
- 🚀 **Herramientas de desarrollo** para flujo de trabajo eficiente

## 📚 Documentación Adicional

- **📋 EJECUCION_ISO.md** - Guía completa de todos los comandos de ejecución
- **💬 Comentarios en código** - Documentación técnica integrada

## 🛡️ Flujo de Trabajo Recomendado

### Primera vez

```bash
make build-clean     # Compilar y generar ISO limpio
```

### Desarrollo

```bash
make run-iso         # Pruebas rápidas
make run-iso-debug   # Si hay problemas
```

### Presentación

```bash
./run.sh --info      # Verificar ISO
./run.sh --gui       # Demo en modo gráfico
```

---

**¡Ahora tienes un sistema operativo completo con herramientas de desarrollo profesionales!** 🎉

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
