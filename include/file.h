#ifndef _FILE_H
#define _FILE_H

#include "stdint.h"

// Constantes para archivos
#define MAX_OPEN_FILES 64 // Global Open File Table (GOFT)
#define MAX_PROCESS_FD 16 // Máx descriptores por proceso
#define EOF (-1)

// Modos de apertura de archivo
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR 0x03
#define O_CREAT 0x04
#define O_TRUNC 0x08
#define O_APPEND 0x10

// --- Tabla global de archivos abiertos ---
typedef struct
{
    uint32_t inode_num; // Número de inodo
    uint32_t position;  // Posición actual en el archivo
    uint32_t flags;     // Banderas de apertura
    uint32_t refcount;  // Cuántos procesos comparten esta entrada
    uint8_t in_use;     // Entrada activa
} global_file_entry_t;

extern global_file_entry_t global_file_table[MAX_OPEN_FILES];

// --- Tabla de FD por proceso ---
typedef struct
{
    int global_index; // Índice en la tabla global
    uint8_t is_open;  // Si el fd está en uso
} process_fd_table_t;

// --- Proceso (solo tabla de FD) ---
typedef struct
{
    process_fd_table_t fds[MAX_PROCESS_FD];
} process_t;

// --- Funciones ---
int file_open(const char *filename, uint32_t flags);
int file_close(int fd);
int file_read(int fd, void *buffer, uint32_t count);
int file_write(int fd, const void *buffer, uint32_t count);
int file_seek(int fd, uint32_t offset, int whence);
int file_tell(int fd);
int file_eof(int fd);

// Constantes para file_seek
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#endif
