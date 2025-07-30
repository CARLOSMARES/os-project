#!/bin/bash
cd /home/carlosmaresdev/dev/os-project
echo "Ejecutando MicroCIOMOS desde ISO..."
qemu-system-x86_64 -cdrom output/os.iso -m 128M -vga std -accel tcg
