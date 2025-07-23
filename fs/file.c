#include "../include/file.h"
#include "../include/fs.h"
#include "../include/stdio.h"

// Acceso al almacenamiento del sistema de archivos
extern uint8_t fs_storage[];

// Tabla global de descriptores de archivo
file_descriptor_t file_table[MAX_OPEN_FILES];
static uint8_t file_system_initialized = 0;

/**
 * Inicializa la tabla de descriptores de archivo
 */
static void init_file_table(void)
{
    if (file_system_initialized)
        return;

    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        file_table[i].inode_num = 0;
        file_table[i].position = 0;
        file_table[i].flags = 0;
        file_table[i].is_open = 0;
    }

    file_system_initialized = 1;
}

/**
 * Busca un descriptor de archivo libre
 */
static int find_free_fd(void)
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (!file_table[i].is_open)
        {
            return i;
        }
    }
    return -1; // No hay descriptores libres
}

/**
 * Abre un archivo
 */
int file_open(const char *filename, uint32_t flags)
{
    if (!filename)
        return -1;

    init_file_table();

    // Buscar un descriptor libre
    int fd = find_free_fd();
    if (fd == -1)
    {
        return -1; // No hay descriptores libres
    }

    uint32_t inode_num;

    // Si el archivo no existe y se especifica O_CREAT, crearlo
    if (fs_find_file(filename, &inode_num) != 0)
    {
        if (flags & O_CREAT)
        {
            if (fs_create_file(filename, FILE_TYPE_REGULAR) != 0)
            {
                return -1; // Error creando archivo
            }
            if (fs_find_file(filename, &inode_num) != 0)
            {
                return -1; // Error encontrando archivo recién creado
            }
        }
        else
        {
            return -1; // Archivo no existe y no se especificó O_CREAT
        }
    }

    // Verificar permisos de acceso
    inode_t *file_inode = fs_get_inode(inode_num);
    if (!file_inode)
    {
        return -1;
    }

    // Inicializar descriptor de archivo
    file_table[fd].inode_num = inode_num;
    file_table[fd].flags = flags;
    file_table[fd].is_open = 1;

    // Establecer posición inicial
    if (flags & O_APPEND)
    {
        file_table[fd].position = file_inode->size;
    }
    else
    {
        file_table[fd].position = 0;
    }

    // Si se especifica O_TRUNC, truncar el archivo
    if (flags & O_TRUNC)
    {
        file_inode->size = 0;
        // Liberar bloques del archivo
        for (int i = 0; i < 12 && file_inode->blocks[i] != 0; i++)
        {
            fs_free_block(file_inode->blocks[i]);
            file_inode->blocks[i] = 0;
        }
        if (file_inode->indirect_block != 0)
        {
            fs_free_block(file_inode->indirect_block);
            file_inode->indirect_block = 0;
        }
    }

    return fd;
}

/**
 * Cierra un archivo
 */
int file_close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open)
    {
        return -1;
    }

    file_table[fd].inode_num = 0;
    file_table[fd].position = 0;
    file_table[fd].flags = 0;
    file_table[fd].is_open = 0;

    return 0;
}

/**
 * Lee datos de un archivo
 */
int file_read(int fd, void *buffer, uint32_t count)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open || !buffer)
    {
        return -1;
    }

    // Verificar permisos de lectura
    if (!(file_table[fd].flags & O_RDONLY) && !(file_table[fd].flags & O_RDWR))
    {
        return -1; // No hay permisos de lectura
    }

    inode_t *file_inode = fs_get_inode(file_table[fd].inode_num);
    if (!file_inode)
    {
        return -1;
    }

    uint32_t position = file_table[fd].position;
    uint32_t file_size = file_inode->size;

    // Ajustar la cantidad a leer si excede el tamaño del archivo
    if (position >= file_size)
    {
        return 0; // EOF
    }

    if (position + count > file_size)
    {
        count = file_size - position;
    }

    uint32_t bytes_read = 0;
    uint8_t *buf = (uint8_t *)buffer;

    while (bytes_read < count)
    {
        uint32_t block_index = position / BLOCK_SIZE;
        uint32_t block_offset = position % BLOCK_SIZE;
        uint32_t bytes_to_read = BLOCK_SIZE - block_offset;

        if (bytes_to_read > (count - bytes_read))
        {
            bytes_to_read = count - bytes_read;
        }

        // Obtener número de bloque
        uint32_t block_num = 0;
        if (block_index < 12)
        {
            block_num = file_inode->blocks[block_index];
        }
        // TODO: Implementar bloques indirectos para archivos grandes

        if (block_num == 0)
        {
            // Bloque no asignado, llenar con ceros
            for (uint32_t i = 0; i < bytes_to_read; i++)
            {
                buf[bytes_read + i] = 0;
            }
        }
        else
        {
            // Leer del bloque
            uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE) + block_offset;
            for (uint32_t i = 0; i < bytes_to_read; i++)
            {
                buf[bytes_read + i] = block_data[i];
            }
        }

        bytes_read += bytes_to_read;
        position += bytes_to_read;
    }

    file_table[fd].position = position;
    return bytes_read;
}

/**
 * Escribe datos en un archivo
 */
int file_write(int fd, const void *buffer, uint32_t count)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open || !buffer)
    {
        return -1;
    }

    // Verificar permisos de escritura
    if (!(file_table[fd].flags & O_WRONLY) && !(file_table[fd].flags & O_RDWR))
    {
        return -1; // No hay permisos de escritura
    }

    inode_t *file_inode = fs_get_inode(file_table[fd].inode_num);
    if (!file_inode)
    {
        return -1;
    }

    uint32_t position = file_table[fd].position;
    uint32_t bytes_written = 0;
    const uint8_t *buf = (const uint8_t *)buffer;

    while (bytes_written < count)
    {
        uint32_t block_index = position / BLOCK_SIZE;
        uint32_t block_offset = position % BLOCK_SIZE;
        uint32_t bytes_to_write = BLOCK_SIZE - block_offset;

        if (bytes_to_write > (count - bytes_written))
        {
            bytes_to_write = count - bytes_written;
        }

        // Obtener o asignar bloque
        uint32_t block_num = 0;
        if (block_index < 12)
        {
            block_num = file_inode->blocks[block_index];
            if (block_num == 0)
            {
                // Asignar nuevo bloque
                block_num = fs_allocate_block();
                if (block_num == 0)
                {
                    break; // No hay bloques libres
                }
                file_inode->blocks[block_index] = block_num;

                // Limpiar el bloque
                uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE);
                for (int i = 0; i < BLOCK_SIZE; i++)
                {
                    block_data[i] = 0;
                }
            }
        }
        // TODO: Implementar bloques indirectos para archivos grandes

        if (block_num == 0)
        {
            break; // No se pudo asignar bloque
        }

        // Escribir en el bloque
        uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE) + block_offset;
        for (uint32_t i = 0; i < bytes_to_write; i++)
        {
            block_data[i] = buf[bytes_written + i];
        }

        bytes_written += bytes_to_write;
        position += bytes_to_write;
    }

    // Actualizar tamaño del archivo si es necesario
    if (position > file_inode->size)
    {
        file_inode->size = position;
    }

    file_table[fd].position = position;
    return bytes_written;
}

/**
 * Cambia la posición del cursor en el archivo
 */
int file_seek(int fd, uint32_t offset, int whence)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open)
    {
        return -1;
    }

    inode_t *file_inode = fs_get_inode(file_table[fd].inode_num);
    if (!file_inode)
    {
        return -1;
    }

    uint32_t new_position;

    switch (whence)
    {
    case SEEK_SET:
        new_position = offset;
        break;
    case SEEK_CUR:
        new_position = file_table[fd].position + offset;
        break;
    case SEEK_END:
        new_position = file_inode->size + offset;
        break;
    default:
        return -1;
    }

    file_table[fd].position = new_position;
    return new_position;
}

/**
 * Obtiene la posición actual del cursor
 */
int file_tell(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open)
    {
        return -1;
    }

    return file_table[fd].position;
}

/**
 * Verifica si se alcanzó el final del archivo
 */
int file_eof(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].is_open)
    {
        return 1; // Error se considera EOF
    }

    inode_t *file_inode = fs_get_inode(file_table[fd].inode_num);
    if (!file_inode)
    {
        return 1;
    }

    return file_table[fd].position >= file_inode->size;
}

// Funciones adicionales para compatibilidad con el sistema de archivos

/**
 * Abre un archivo (wrapper de fs)
 */
int fs_open_file(const char *filename)
{
    return file_open(filename, O_RDWR);
}

/**
 * Cierra un archivo (wrapper de fs)
 */
int fs_close_file(int fd)
{
    return file_close(fd);
}

/**
 * Lee de un archivo con offset específico
 */
int fs_read_file(int fd, void *buffer, uint32_t size, uint32_t offset)
{
    if (file_seek(fd, offset, SEEK_SET) == -1)
    {
        return -1;
    }
    return file_read(fd, buffer, size);
}

/**
 * Escribe en un archivo con offset específico
 */
int fs_write_file(int fd, const void *buffer, uint32_t size, uint32_t offset)
{
    if (file_seek(fd, offset, SEEK_SET) == -1)
    {
        return -1;
    }
    return file_write(fd, buffer, size);
}