#ifndef _FS_H
#define _FS_H

#include "stdint.h"
#include "sys/types.h"

// Constantes del sistema de archivos
#define MAX_FILENAME 32
#define MAX_FILES 64
#define BLOCK_SIZE 512
#define MAX_BLOCKS 1024
#define INODE_SIZE 64

// Número mágico del sistema de archivos
#define FS_MAGIC 0x12345678

// Tipos de archivo
#define FILE_TYPE_REGULAR 1
#define FILE_TYPE_DIRECTORY 2

// Estados de bloque
#define BLOCK_FREE 0
#define BLOCK_USED 1

// Códigos de error
#define FS_SUCCESS 0
#define FS_ERROR_NOT_FOUND -1
#define FS_ERROR_NO_SPACE -2
#define FS_ERROR_INVALID_PARAM -3
#define FS_ERROR_ALREADY_EXISTS -4
#define FS_ERROR_NOT_INITIALIZED -5

// Estructura del superbloque
typedef struct
{
    uint32_t magic;            // Número mágico para identificar el FS
    uint32_t total_blocks;     // Total de bloques en el sistema
    uint32_t free_blocks;      // Bloques libres
    uint32_t total_inodes;     // Total de inodos
    uint32_t free_inodes;      // Inodos libres
    uint32_t first_data_block; // Primer bloque de datos
    uint32_t block_size;       // Tamaño de bloque
    uint32_t inode_size;       // Tamaño de inodo
} superblock_t;

// Estructura del inodo
typedef struct
{
    uint32_t size;           // Tamaño del archivo en bytes
    uint32_t type;           // Tipo de archivo
    uint32_t blocks[12];     // Bloques directos
    uint32_t indirect_block; // Bloque indirecto
    uint32_t created_time;   // Tiempo de creación
    uint32_t modified_time;  // Tiempo de modificación
    uint32_t permissions;    // Permisos del archivo
    uint32_t links;          // Número de enlaces
} inode_t;

// Entrada de directorio
typedef struct
{
    uint32_t inode_num;          // Número de inodo
    char filename[MAX_FILENAME]; // Nombre del archivo
    uint8_t file_type;           // Tipo de archivo
    uint8_t name_len;            // Longitud del nombre
} dir_entry_t;

// Mapa de bits para bloques libres
typedef struct
{
    uint8_t bitmap[MAX_BLOCKS / 8];
} block_bitmap_t;

// Mapa de bits para inodos libres
typedef struct
{
    uint8_t bitmap[MAX_FILES / 8];
} inode_bitmap_t;

// Funciones del sistema de archivos
int fs_init(void);
int fs_format(void);
int fs_create_file(const char *filename, uint32_t type);
int fs_delete_file(const char *filename);
int fs_open_file(const char *filename);
int fs_close_file(int fd);
int fs_read_file(int fd, void *buffer, uint32_t size, uint32_t offset);
int fs_write_file(int fd, const void *buffer, uint32_t size, uint32_t offset);
int fs_get_file_size(const char *filename);
int fs_list_directory(const char *dirname, dir_entry_t *entries, uint32_t max_entries);

// Funciones internas
uint32_t fs_allocate_block(void);
void fs_free_block(uint32_t block_num);
uint32_t fs_allocate_inode(void);
void fs_free_inode(uint32_t inode_num);
inode_t *fs_get_inode(uint32_t inode_num);
int fs_find_file(const char *filename, uint32_t *inode_num);

#endif
