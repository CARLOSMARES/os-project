#include "../include/stdio.h"
#include "../include/stdint.h"
#include "../include/fs.h"
#include "../include/file.h"

// Declaraciones externas
extern void vga_initialize(void);
extern void vga_set_color(uint8_t color);
extern void kernel_main(void);

// Funciones de inicialización
void init_system(void);
void init_filesystem(void);
void init_hardware(void);
void show_boot_info(void);

/**
 * Función principal del sistema - Punto de entrada
 * Esta función se llama después del bootloader y configura todo el sistema
 */
__attribute__((used)) void main(void)
{
    // Mostrar información de arranque
    show_boot_info();

    // Inicializar hardware básico
    init_hardware();

    // Inicializar sistema de archivos
    init_filesystem();

    // Inicializar otros componentes del sistema
    init_system();

    // Transferir control al kernel principal
    printf("\n[INIT] Transfiriendo control al kernel...\n");
    kernel_main();

    // Si kernel_main retorna, detener el sistema
    printf("\n[INIT] Sistema detenido.\n");
    while (1)
    {
        __asm__ volatile("hlt");
    }
}

/**
 * Muestra información del arranque del sistema
 */
void show_boot_info(void)
{
    // Inicializar VGA primero para poder mostrar texto
    vga_initialize();

    printf("=====================================\n");
    printf("        MicroCIOMOS v1.0            \n");
    printf("=====================================\n");
    printf("\n[INIT] Iniciando sistema operativo...\n");
    printf("[INIT] Arquitectura: x86_64\n");
    printf("[INIT] Compilado: %s %s\n", __DATE__, __TIME__);
    printf("\n");
}

/**
 * Inicializa el hardware básico del sistema
 */
void init_hardware(void)
{
    printf("[INIT] Inicializando hardware...\n");

    // VGA ya está inicializado desde show_boot_info()
    printf("  [OK] VGA inicializado\n");

    // Aquí se pueden agregar más componentes de hardware
    // TODO: Inicializar timer, keyboard, etc.

    printf("  [OK] Hardware básico inicializado\n");
}

/**
 * Inicializa el sistema de archivos
 */
void init_filesystem(void)
{
    printf("[INIT] Inicializando sistema de archivos...\n");

    // Inicializar el sistema de archivos
    if (fs_init() == 0)
    {
        printf("  [OK] Sistema de archivos inicializado\n");

        // Crear algunos archivos del sistema por defecto
        if (fs_create_file("boot.log", FILE_TYPE_REGULAR) == 0)
        {
            printf("  [OK] Archivo boot.log creado\n");

            // Escribir log de arranque
            int fd = file_open("boot.log", O_RDWR | O_CREAT);
            if (fd >= 0)
            {
                const char *boot_msg = "Sistema iniciado correctamente\nSistema de archivos operativo\n";
                file_write(fd, boot_msg, 52);
                file_close(fd);
                printf("  [OK] Log de arranque escrito\n");
            }
        }

        // Crear directorio de configuración (simulado como archivo por ahora)
        if (fs_create_file("config.sys", FILE_TYPE_REGULAR) == 0)
        {
            printf("  [OK] Archivo de configuración creado\n");

            int fd = file_open("config.sys", O_RDWR | O_CREAT);
            if (fd >= 0)
            {
                const char *config = "# Configuracion del sistema\nversion=1.0\ndebug=true\n";
                file_write(fd, config, 48);
                file_close(fd);
            }
        }
    }
    else
    {
        printf("  [ERROR] Fallo al inicializar sistema de archivos\n");
    }
}

/**
 * Inicializa otros componentes del sistema
 */
void init_system(void)
{
    printf("[INIT] Inicializando componentes del sistema...\n");

    // Aquí se pueden inicializar otros subsistemas
    // TODO: Procesos, memoria, scheduler, etc.

    printf("  [OK] Componentes del sistema inicializados\n");

    // Mostrar estadísticas del sistema
    printf("\n[INIT] Estadísticas del sistema:\n");
    printf("  - Memoria disponible: ~1MB\n");
    printf("  - Sistema de archivos: Activo\n");
    printf("  - Archivos del sistema: Creados\n");

    // Listar archivos creados
    printf("\n[INIT] Archivos del sistema:\n");
    dir_entry_t entries[10];
    int count = fs_list_directory("/", entries, 10);

    for (int i = 0; i < count; i++)
    {
        const char *type_str = (entries[i].file_type == FILE_TYPE_REGULAR) ? "archivo" : "directorio";
        int size = fs_get_file_size(entries[i].filename);
        printf("  - %s (%s, %d bytes)\n", entries[i].filename, type_str, size);
    }

    printf("\n[INIT] Sistema listo.\n");
}

/**
 * Función de emergencia en caso de panic del sistema
 */
void system_panic(const char *message)
{
    printf("\n!!! PANIC DEL SISTEMA !!!\n");
    printf("Mensaje: %s\n", message);
    printf("Sistema detenido por seguridad.\n");

    while (1)
    {
        __asm__ volatile("hlt");
    }
}
