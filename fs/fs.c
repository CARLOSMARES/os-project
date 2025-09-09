#include "fs.h"
#include "stdio.h"

// Variables globales del sistema de archivos
static superblock_t superblock;
static block_bitmap_t block_bitmap;
static inode_bitmap_t inode_bitmap;
static inode_t inodes[MAX_FILES];
uint8_t fs_storage[MAX_BLOCKS * BLOCK_SIZE]; // No static para acceso desde file.c
static uint8_t fs_initialized = 0;

// Número mágico para identificar nuestro FS
#define FS_MAGIC 0x12345678

// Prototipo privado
static void fs_save_metadata(void);

/**
 * Inicializa el sistema de archivos
 */
int fs_init(void)
{
    if (fs_initialized)
        return 0; // Ya inicializado

    // Verificar si ya existe un sistema de archivos válido
    superblock_t *sb = (superblock_t *)fs_storage;
    if (sb->magic == FS_MAGIC)
    {
        // Cargar sistema de archivos existente
        superblock = *sb;

        // Cargar bitmaps desde el almacenamiento
        uint8_t *bitmap_area = fs_storage + BLOCK_SIZE;
        memcpy(&block_bitmap, bitmap_area, sizeof(block_bitmap));
        bitmap_area += sizeof(block_bitmap);
        memcpy(&inode_bitmap, bitmap_area, sizeof(inode_bitmap));

        // Cargar tabla de inodos
        uint8_t *inode_area = bitmap_area + sizeof(inode_bitmap);
        memcpy(inodes, inode_area, sizeof(inodes));
    }
    else
    {
        // Formatear nuevo sistema de archivos
        fs_format();
    }

    fs_initialized = 1;
    return 0;
}

/**
 * Guarda los metadatos en memoria
 */
static void fs_save_metadata(void)
{
    // Guardar superbloque
    *(superblock_t *)fs_storage = superblock;

    // Guardar bitmaps
    uint8_t *bitmap_area = fs_storage + BLOCK_SIZE;
    memcpy(bitmap_area, &block_bitmap, sizeof(block_bitmap));
    bitmap_area += sizeof(block_bitmap);
    memcpy(bitmap_area, &inode_bitmap, sizeof(inode_bitmap));

    // Guardar inodos
    uint8_t *inode_area = bitmap_area + sizeof(inode_bitmap);
    memcpy(inode_area, inodes, sizeof(inodes));
}

/**
 * Formatea el sistema de archivos
 */
int fs_format(void)
{
    memset(fs_storage, 0, sizeof(fs_storage));

    // Inicializar superbloque
    superblock.magic = FS_MAGIC;
    superblock.total_blocks = MAX_BLOCKS;
    superblock.free_blocks = MAX_BLOCKS - 10; // Reservar bloques para metadatos
    superblock.total_inodes = MAX_FILES;
    superblock.free_inodes = MAX_FILES - 1; // Reservar inodo 0 para directorio raíz
    superblock.first_data_block = 10;
    superblock.block_size = BLOCK_SIZE;
    superblock.inode_size = INODE_SIZE;

    // Inicializar bitmaps
    memset(block_bitmap.bitmap, 0, sizeof(block_bitmap.bitmap));
    memset(inode_bitmap.bitmap, 0, sizeof(inode_bitmap.bitmap));

    // Reservar bloques de metadatos (0-9)
    for (int i = 0; i < 10; i++)
        block_bitmap.bitmap[i / 8] |= (1 << (i % 8));

    // Reservar inodo 0 (directorio raíz)
    inode_bitmap.bitmap[0] |= 1;

    // Inicializar inodos
    memset(inodes, 0, sizeof(inodes));

    // Crear directorio raíz
    inodes[0].type = FILE_TYPE_DIRECTORY;
    inodes[0].size = 0;
    inodes[0].links = 1;
    inodes[0].permissions = 0755;

    // Guardar metadatos
    fs_save_metadata();

    fs_initialized = 1;
    return 0;
}

/**
 * Asigna un bloque libre
 */
uint32_t fs_allocate_block(void)
{
    for (uint32_t i = superblock.first_data_block; i < superblock.total_blocks; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(block_bitmap.bitmap[byte_index] & (1 << bit_index)))
        {
            // Bloque libre encontrado
            block_bitmap.bitmap[byte_index] |= (1 << bit_index);
            superblock.free_blocks--;

            fs_save_metadata(); // Persistir cambios
            return i;
        }
    }
    return 0; // No hay bloques libres
}

/**
 * Libera un bloque
 */
void fs_free_block(uint32_t block_num)
{
    if (block_num >= superblock.total_blocks)
        return;

    int byte_index = block_num / 8;
    int bit_index = block_num % 8;

    block_bitmap.bitmap[byte_index] &= ~(1 << bit_index);
    superblock.free_blocks++;

    fs_save_metadata(); // Persistir cambios
}

/**
 * Asigna un inodo libre
 */
uint32_t fs_allocate_inode(void)
{
    for (uint32_t i = 1; i < superblock.total_inodes; i++)
    { // Empezar desde 1 (0 es directorio raíz)
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(inode_bitmap.bitmap[byte_index] & (1 << bit_index)))
        {
            // Inodo libre encontrado
            inode_bitmap.bitmap[byte_index] |= (1 << bit_index);
            superblock.free_inodes--;

            fs_save_metadata(); // Persistir cambios
            return i;
        }
    }
    return 0; // No hay inodos libres
}

/**
 * Libera un inodo
 */
void fs_free_inode(uint32_t inode_num)
{
    if (inode_num >= superblock.total_inodes || inode_num == 0)
        return;

    int byte_index = inode_num / 8;
    int bit_index = inode_num % 8;

    inode_bitmap.bitmap[byte_index] &= ~(1 << bit_index);
    superblock.free_inodes++;

    // Limpiar el inodo
    memset(&inodes[inode_num], 0, sizeof(inode_t));

    fs_save_metadata(); // Persistir cambios
}

/**
 * Crea un nuevo archivo
 */
int fs_create_file(const char *filename, uint32_t type)
{
    if (!fs_initialized)
        fs_init();

    if (!filename)
        return -1;

    // Verificar si ya existe
    uint32_t existing_inode;
    if (fs_find_file(filename, &existing_inode) == 0)
        return -1;

    // Asignar inodo
    uint32_t new_inode_num = fs_allocate_inode();
    if (new_inode_num == 0)
        return -1;

    inode_t *new_inode = fs_get_inode(new_inode_num);
    new_inode->type = type;
    new_inode->size = 0;
    new_inode->links = 1;
    new_inode->permissions = 0644;

    // Agregar entrada al root
    inode_t *root_inode = fs_get_inode(0);
    if (root_inode->blocks[0] == 0)
    {
        uint32_t new_block = fs_allocate_block();
        if (new_block == 0)
        {
            fs_free_inode(new_inode_num);
            return -1;
        }
        root_inode->blocks[0] = new_block;
        root_inode->size = 0;
    }

    for (int block_idx = 0; block_idx < 12 && root_inode->blocks[block_idx] != 0; block_idx++)
    {
        uint32_t block_num = root_inode->blocks[block_idx];
        dir_entry_t *entries = (dir_entry_t *)(fs_storage + block_num * BLOCK_SIZE);

        int entries_per_block = BLOCK_SIZE / sizeof(dir_entry_t);
        for (int i = 0; i < entries_per_block; i++)
        {
            if (entries[i].inode_num == 0)
            {
                entries[i].inode_num = new_inode_num;
                entries[i].file_type = type;

                int name_len = 0;
                while (filename[name_len] != '\0' && name_len < MAX_FILENAME - 1)
                {
                    entries[i].filename[name_len] = filename[name_len];
                    name_len++;
                }
                entries[i].filename[name_len] = '\0';
                entries[i].name_len = name_len;

                root_inode->size += sizeof(dir_entry_t);

                fs_save_metadata(); // Persistir cambios
                return 0;
            }
        }
    }

    fs_free_inode(new_inode_num);
    return -1;
}

/**
 * Elimina un archivo
 */
int fs_delete_file(const char *filename)
{
    if (!fs_initialized)
        fs_init();

    uint32_t inode_num;
    if (fs_find_file(filename, &inode_num) != 0)
        return -1;

    inode_t *file_inode = fs_get_inode(inode_num);
    if (!file_inode)
        return -1;

    for (int i = 0; i < 12 && file_inode->blocks[i] != 0; i++)
        fs_free_block(file_inode->blocks[i]);

    if (file_inode->indirect_block != 0)
        fs_free_block(file_inode->indirect_block);

    fs_free_inode(inode_num);

    inode_t *root_inode = fs_get_inode(0);
    for (int block_idx = 0; block_idx < 12 && root_inode->blocks[block_idx] != 0; block_idx++)
    {
        uint32_t block_num = root_inode->blocks[block_idx];
        dir_entry_t *entries = (dir_entry_t *)(fs_storage + block_num * BLOCK_SIZE);

        int entries_per_block = BLOCK_SIZE / sizeof(dir_entry_t);
        for (int i = 0; i < entries_per_block; i++)
        {
            if (entries[i].inode_num == inode_num)
            {
                entries[i].inode_num = 0;
                root_inode->size -= sizeof(dir_entry_t);

                fs_save_metadata(); // Persistir cambios
                return 0;
            }
        }
    }

    return 0;
}
