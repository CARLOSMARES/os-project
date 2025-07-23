#!/bin/bash

# Script para ejecutar el ISO de MicroCIOMOS
# Autor: Proyecto MicroCIOMOS
# Fecha: $(date)

# Configuración
ISO_FILE="os.iso"
QEMU_CMD="qemu-system-x86_64"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Función para mostrar ayuda
show_help() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE}        MicroCIOMOS - Script de Ejecución${NC}"
    echo -e "${BLUE}================================================${NC}"
    echo ""
    echo "Uso: $0 [OPCIÓN]"
    echo ""
    echo "Opciones:"
    echo "  --help, -h        Mostrar esta ayuda"
    echo "  --normal, -n      Ejecutar en modo normal (por defecto)"
    echo "  --debug, -d       Ejecutar con debug habilitado"
    echo "  --gui, -g         Ejecutar en modo gráfico mejorado"
    echo "  --info, -i        Mostrar información del ISO"
    echo ""
    echo "Ejemplos:"
    echo "  $0                # Ejecutar en modo normal"
    echo "  $0 --debug       # Ejecutar con debug"
    echo "  $0 --gui         # Ejecutar en modo gráfico"
    echo ""
    echo -e "${BLUE}================================================${NC}"
}

# Función para verificar si existe QEMU
check_qemu() {
    if ! command -v $QEMU_CMD &> /dev/null; then
        echo -e "${RED}[!] Error: $QEMU_CMD no está instalado${NC}"
        echo -e "${YELLOW}[•] Para instalar QEMU en Ubuntu/Debian:${NC}"
        echo "    sudo apt update && sudo apt install qemu-system-x86"
        echo -e "${YELLOW}[•] Para instalar QEMU en Fedora/CentOS:${NC}"
        echo "    sudo dnf install qemu-system-x86"
        exit 1
    fi
}

# Función para verificar si existe el ISO
check_iso() {
    if [ ! -f "$ISO_FILE" ]; then
        echo -e "${RED}[!] Error: No se encontró $ISO_FILE${NC}"
        echo -e "${YELLOW}[•] Asegúrate de estar en el directorio correcto del proyecto${NC}"
        echo -e "${YELLOW}[•] O ejecuta 'make all' o 'make build-clean' para crear el ISO${NC}"
        exit 1
    fi
}

# Función para mostrar información del ISO
show_info() {
    if [ -f "$ISO_FILE" ]; then
        echo -e "${GREEN}[✓] Información del ISO:${NC}"
        echo "  Archivo: $ISO_FILE"
        echo "  Tamaño: $(du -h $ISO_FILE | cut -f1)"
        echo "  Fecha: $(date -r $ISO_FILE)"
        echo ""
        echo -e "${BLUE}[•] El ISO está listo para ejecutar${NC}"
    else
        echo -e "${RED}[!] No se encontró el archivo $ISO_FILE${NC}"
        exit 1
    fi
}

# Función para ejecutar en modo normal
run_normal() {
    echo -e "${GREEN}[✓] Ejecutando $ISO_FILE en modo normal...${NC}"
    $QEMU_CMD -cdrom $ISO_FILE
}

# Función para ejecutar en modo debug
run_debug() {
    echo -e "${GREEN}[✓] Ejecutando $ISO_FILE con debug habilitado...${NC}"
    echo -e "${YELLOW}[•] Debug: Monitor de QEMU habilitado (usar 'info registers' para ver estado)${NC}"
    $QEMU_CMD -cdrom $ISO_FILE -monitor stdio -d int,cpu_reset
}

# Función para ejecutar en modo gráfico
run_gui() {
    echo -e "${GREEN}[✓] Ejecutando $ISO_FILE en modo gráfico mejorado...${NC}"
    $QEMU_CMD -cdrom $ISO_FILE -vga std -display gtk
}

# Función principal
main() {
    # Verificar dependencias
    check_qemu
    check_iso
    
    # Procesar argumentos
    case "${1:-}" in
        --help|-h)
            show_help
            exit 0
            ;;
        --info|-i)
            show_info
            exit 0
            ;;
        --debug|-d)
            run_debug
            ;;
        --gui|-g)
            run_gui
            ;;
        --normal|-n|"")
            run_normal
            ;;
        *)
            echo -e "${RED}[!] Opción desconocida: $1${NC}"
            echo -e "${YELLOW}[•] Use --help para ver las opciones disponibles${NC}"
            exit 1
            ;;
    esac
}

# Ejecutar función principal con todos los argumentos
main "$@"
