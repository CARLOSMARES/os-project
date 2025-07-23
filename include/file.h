#ifndef _FILE_H
#define _FILE_H

#include "stdint.h"

// Constantes para archivos
#define MAX_OPEN_FILES 16
#define EOF (-1)

// Modos de apertura de archivo
#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR 0x03
#define O_CREAT 0x04
#define O_TRUNC 0x08
#define O_APPEND 0x10

// Estructura de descriptor de archivo
typedef struct
{
    uint32_t inode_num; // Número de inodo
    uint32_t position;  // Posición actual en el archivo
    uint32_t flags;     // Banderas de apertura
    uint8_t is_open;    // Si el archivo está abierto
} file_descriptor_t;

// Tabla de descriptores de archivo
extern file_descriptor_t file_table[MAX_OPEN_FILES];

// Funciones para manejo de archivos
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
