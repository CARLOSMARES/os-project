#include "fs.h"
#include "ahci.h"
#include "stdio.h"
#include "string.h"
#include "file.h"

// Variables globales del sistema de archivos
static superblock_t superblock;
static block_bitmap_t block_bitmap;
static inode_bitmap_t inode_bitmap;
static inode_t inodes[MAX_FILES];

// Driver AHCI global
extern ahci_device_t *ahci_dev = NULL;

// Flag de inicialización
static uint8_t fs_initialized = 0;

// Tabla de archivos abiertos
static global_file_entry_t file_table[MAX_OPEN_FILES];

// --- Funciones internas de lectura/escritura de bloques ---
static void fs_read_block(uint32_t block_num, void *buffer)
{
    ahci_read_block(&ahci_dev, block_num, buffer);
}

static void fs_write_block(uint32_t block_num, const void *buffer)
{
    ahci_write_block(&ahci_dev, block_num, buffer);
}

// --- Guardar y cargar metadatos ---
static void fs_save_metadata(void)
{
    uint8_t buffer[BLOCK_SIZE];

    fs_write_block(0, &superblock);
    fs_write_block(1, &block_bitmap);
    fs_write_block(2, &inode_bitmap);

    int inodes_per_block = BLOCK_SIZE / sizeof(inode_t);
    for (int i = 0; i < MAX_FILES; i++)
    {
        int blk = 3 + i / inodes_per_block;
        int offset = i % inodes_per_block;

        if (offset == 0)
            memset(buffer, 0, BLOCK_SIZE);

        memcpy(buffer + offset * sizeof(inode_t), &inodes[i], sizeof(inode_t));

        if (offset == inodes_per_block - 1 || i == MAX_FILES - 1)
            fs_write_block(blk, buffer);
    }
}

static void fs_load_metadata(void)
{
    uint8_t buffer[BLOCK_SIZE];

    fs_read_block(0, &superblock);

    if (superblock.magic != FS_MAGIC)
        return;

    fs_read_block(1, &block_bitmap);
    fs_read_block(2, &inode_bitmap);

    int inodes_per_block = BLOCK_SIZE / sizeof(inode_t);
    for (int i = 0; i < MAX_FILES; i++)
    {
        int blk = 3 + i / inodes_per_block;
        int offset = i % inodes_per_block;

        if (offset == 0)
            fs_read_block(blk, buffer);

        memcpy(&inodes[i], buffer + offset * sizeof(inode_t), sizeof(inode_t));
    }
}

// --- Inicialización del FS ---
int fs_init(void)
{
    if (fs_initialized)
        return FS_SUCCESS;

    fs_load_metadata();

    if (superblock.magic != FS_MAGIC)
    {
        printf("Formateando FS...\n");
        return fs_format();
    }

    fs_initialized = 1;

    // Inicializar tabla de archivos
    for (int i = 0; i < MAX_OPEN_FILES; i++)
        file_table[i].in_use = 0;

    return FS_SUCCESS;
}

// --- Formateo del FS ---
int fs_format(void)
{
    memset(&superblock, 0, sizeof(superblock));
    memset(&block_bitmap, 0, sizeof(block_bitmap));
    memset(&inode_bitmap, 0, sizeof(inode_bitmap));
    memset(inodes, 0, sizeof(inodes));

    superblock.magic = FS_MAGIC;
    superblock.total_blocks = MAX_BLOCKS;
    superblock.free_blocks = MAX_BLOCKS - 10;
    superblock.total_inodes = MAX_FILES;
    superblock.free_inodes = MAX_FILES - 1;
    superblock.first_data_block = 10;
    superblock.block_size = BLOCK_SIZE;
    superblock.inode_size = INODE_SIZE;

    for (int i = 0; i < 10; i++)
        block_bitmap.bitmap[i / 8] |= (1 << (i % 8));

    inode_bitmap.bitmap[0] |= 1;
    inodes[0].type = FILE_TYPE_DIRECTORY;
    inodes[0].size = 0;
    inodes[0].links = 1;
    inodes[0].permissions = 0755;

    fs_save_metadata();
    fs_initialized = 1;

    printf("FS formateado correctamente\n");
    return FS_SUCCESS;
}

// --- Bloques/Inodos ---
uint32_t fs_allocate_block(void)
{
    for (uint32_t i = superblock.first_data_block; i < superblock.total_blocks; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;
        if (!(block_bitmap.bitmap[byte_index] & (1 << bit_index)))
        {
            block_bitmap.bitmap[byte_index] |= (1 << bit_index);
            superblock.free_blocks--;
            fs_save_metadata();
            return i;
        }
    }
    return 0;
}

void fs_free_block(uint32_t block_num)
{
    if (block_num >= superblock.total_blocks)
        return;
    int byte_index = block_num / 8;
    int bit_index = block_num % 8;

    block_bitmap.bitmap[byte_index] &= ~(1 << bit_index);
    superblock.free_blocks++;
    fs_save_metadata();
}

uint32_t fs_allocate_inode(void)
{
    for (uint32_t i = 1; i < superblock.total_inodes; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(inode_bitmap.bitmap[byte_index] & (1 << bit_index)))
        {
            inode_bitmap.bitmap[byte_index] |= (1 << bit_index);
            superblock.free_inodes--;
            fs_save_metadata();
            return i;
        }
    }
    return 0;
}

void fs_free_inode(uint32_t inode_num)
{
    if (inode_num == 0 || inode_num >= superblock.total_inodes)
        return;

    int byte_index = inode_num / 8;
    int bit_index = inode_num % 8;

    inode_bitmap.bitmap[byte_index] &= ~(1 << bit_index);
    superblock.free_inodes++;

    memset(&inodes[inode_num], 0, sizeof(inode_t));
    fs_save_metadata();
}

// --- Obtener inodo ---
inode_t *fs_get_inode(uint32_t inode_num)
{
    if (inode_num >= superblock.total_inodes)
        return NULL;
    return &inodes[inode_num];
}

// --- Funciones de archivo ---
int fs_find_file(const char *filename, uint32_t *inode_num)
{
    inode_t *root_inode = fs_get_inode(0);
    if (!root_inode)
        return FS_ERROR_NOT_INITIALIZED;

    for (int block_idx = 0; block_idx < 12 && root_inode->blocks[block_idx] != 0; block_idx++)
    {
        uint8_t buffer[BLOCK_SIZE];
        fs_read_block(root_inode->blocks[block_idx], buffer);

        dir_entry_t *entries = (dir_entry_t *)buffer;
        int entries_per_block = BLOCK_SIZE / sizeof(dir_entry_t);

        for (int i = 0; i < entries_per_block; i++)
        {
            if (entries[i].inode_num != 0 && strcmp(entries[i].filename, filename) == 0)
            {
                *inode_num = entries[i].inode_num;
                return FS_SUCCESS;
            }
        }
    }
    return FS_ERROR_NOT_FOUND;
}

int fs_create_file(const char *filename, uint32_t type)
{
    if (!fs_initialized)
        fs_init();
    if (!filename)
        return FS_ERROR_INVALID_PARAM;

    uint32_t existing_inode;
    if (fs_find_file(filename, &existing_inode) == FS_SUCCESS)
        return FS_ERROR_ALREADY_EXISTS;

    uint32_t new_inode_num = fs_allocate_inode();
    if (new_inode_num == 0)
        return FS_ERROR_NO_SPACE;

    inode_t *new_inode = fs_get_inode(new_inode_num);
    new_inode->type = type;
    new_inode->size = 0;
    new_inode->links = 1;
    new_inode->permissions = 0644;

    inode_t *root_inode = fs_get_inode(0);
    uint8_t block_buf[BLOCK_SIZE];

    for (int blk_idx = 0; blk_idx < 12; blk_idx++)
    {
        uint32_t blk_num = root_inode->blocks[blk_idx];
        if (blk_num == 0)
        {
            blk_num = fs_allocate_block();
            if (blk_num == 0)
            {
                fs_free_inode(new_inode_num);
                return FS_ERROR_NO_SPACE;
            }
            root_inode->blocks[blk_idx] = blk_num;
            memset(block_buf, 0, BLOCK_SIZE);
        }
        else
            fs_read_block(blk_num, block_buf);

        dir_entry_t *entries = (dir_entry_t *)block_buf;
        int entries_per_block = BLOCK_SIZE / sizeof(dir_entry_t);

        for (int i = 0; i < entries_per_block; i++)
        {
            if (entries[i].inode_num == 0)
            {
                entries[i].inode_num = new_inode_num;
                entries[i].file_type = type;
                strncpy(entries[i].filename, filename, MAX_FILENAME - 1);
                entries[i].filename[MAX_FILENAME - 1] = '\0';
                entries[i].name_len = strlen(entries[i].filename);

                fs_write_block(root_inode->blocks[blk_idx], block_buf);
                root_inode->size += sizeof(dir_entry_t);
                fs_save_metadata();
                return FS_SUCCESS;
            }
        }
    }

    fs_free_inode(new_inode_num);
    return FS_ERROR_NO_SPACE;
}

int fs_read_file(int fd, void *buffer, uint32_t size, uint32_t offset)
{
    if (!fs_initialized)
        fs_init();
    inode_t *file_inode = fs_get_inode(fd);
    if (!file_inode)
        return FS_ERROR_NOT_FOUND;

    if (offset >= file_inode->size)
        return 0;
    if (offset + size > file_inode->size)
        size = file_inode->size - offset;

    uint8_t *buf = (uint8_t *)buffer;
    uint32_t bytes_read = 0;

    while (bytes_read < size)
    {
        uint32_t block_index = offset / BLOCK_SIZE;
        uint32_t block_offset = offset % BLOCK_SIZE;
        uint32_t bytes_to_read = BLOCK_SIZE - block_offset;
        if (bytes_to_read > size - bytes_read)
            bytes_to_read = size - bytes_read;

        uint32_t block_num = block_index < 12 ? file_inode->blocks[block_index] : 0;
        uint8_t block_buf[BLOCK_SIZE];

        if (block_num != 0)
        {
            fs_read_block(block_num, block_buf);
            memcpy(buf + bytes_read, block_buf + block_offset, bytes_to_read);
        }
        else
            memset(buf + bytes_read, 0, bytes_to_read);

        bytes_read += bytes_to_read;
        offset += bytes_to_read;
    }

    return bytes_read;
}

int fs_write_file(int fd, const void *buffer, uint32_t size, uint32_t offset)
{
    if (!fs_initialized)
        fs_init();
    inode_t *file_inode = fs_get_inode(fd);
    if (!file_inode)
        return FS_ERROR_NOT_FOUND;

    const uint8_t *buf = (const uint8_t *)buffer;
    uint32_t bytes_written = 0;

    while (bytes_written < size)
    {
        uint32_t block_index = offset / BLOCK_SIZE;
        uint32_t block_offset = offset % BLOCK_SIZE;
        uint32_t bytes_to_write = BLOCK_SIZE - block_offset;
        if (bytes_to_write > size - bytes_written)
            bytes_to_write = size - bytes_written;

        uint32_t block_num = file_inode->blocks[block_index];
        if (block_num == 0)
        {
            block_num = fs_allocate_block();
            if (block_num == 0)
                break;
            file_inode->blocks[block_index] = block_num;
        }

        uint8_t block_buf[BLOCK_SIZE];
        if (block_offset != 0 || bytes_to_write < BLOCK_SIZE)
            fs_read_block(block_num, block_buf);

        memcpy(block_buf + block_offset, buf + bytes_written, bytes_to_write);
        fs_write_block(block_num, block_buf);

        bytes_written += bytes_to_write;
        offset += bytes_to_write;
    }

    if (offset > file_inode->size)
        file_inode->size = offset;
    fs_save_metadata();
    return bytes_written;
}

// --- Tabla de archivos ---
int file_open(const char *filename, uint32_t flags)
{
    if (!fs_initialized)
        fs_init();
    uint32_t inode_num;
    int exists = (fs_find_file(filename, &inode_num) == FS_SUCCESS);

    if (!exists && (flags & O_CREAT))
    {
        if (fs_create_file(filename, FILE_TYPE_REGULAR) != FS_SUCCESS)
            return -1;
        fs_find_file(filename, &inode_num);
    }
    else if (!exists)
        return -1;

    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (!file_table[i].in_use)
        {
            file_table[i].inode_num = inode_num;
            file_table[i].position = 0;
            file_table[i].flags = flags;
            file_table[i].in_use = 1;
            return i;
        }
    }
    return -1;
}

int file_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return -1;
    file_table[fd].in_use = 0;
    return 0;
}

int file_read(int fd, void *buffer, uint32_t count)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return -1;
    int bytes = fs_read_file(file_table[fd].inode_num, buffer, count, file_table[fd].position);
    file_table[fd].position += bytes;
    return bytes;
}

int file_write(int fd, const void *buffer, uint32_t count)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return -1;
    if (!(file_table[fd].flags & (O_WRONLY | O_RDWR)))
        return -1;
    int bytes = fs_write_file(file_table[fd].inode_num, buffer, count, file_table[fd].position);
    file_table[fd].position += bytes;
    return bytes;
}

int file_seek(int fd, uint32_t offset, int whence)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return -1;
    inode_t *inode = fs_get_inode(file_table[fd].inode_num);
    if (!inode)
        return -1;

    switch (whence)
    {
    case SEEK_SET:
        file_table[fd].position = offset;
        break;
    case SEEK_CUR:
        file_table[fd].position += offset;
        break;
    case SEEK_END:
        file_table[fd].position = inode->size + offset;
        break;
    default:
        return -1;
    }

    if (file_table[fd].position > inode->size)
        file_table[fd].position = inode->size;

    return file_table[fd].position;
}

int file_tell(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return -1;
    return file_table[fd].position;
}

int file_eof(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].in_use)
        return 1;
    inode_t *inode = fs_get_inode(file_table[fd].inode_num);
    if (!inode)
        return 1;
    return file_table[fd].position >= inode->size;
}
