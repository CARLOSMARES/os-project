#include "fs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Declaración externa de kernel_main
    extern void kernel_main(void);

    // Punto de entrada del kernel
    __attribute__((section(".entry"), used, naked)) void _start(void)
    {
        fs_init();     // Inicializa el sistema de archivos mínimo necesario
        kernel_main(); // Llama directamente al kernel principal
    }

#ifdef __cplusplus
}
#endif
