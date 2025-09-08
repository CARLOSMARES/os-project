#include "../include/file.h"
#include "../include/fs.h"
#include "../include/stdio.h"

// Acceso al almacenamiento del sistema de archivos
extern uint8_t fs_storage[];

// Tabla global de archivos abiertos
global_file_entry_t global_file_table[MAX_OPEN_FILES];

// Inicializa la tabla global
static void init_global_table(void) {
    static uint8_t initialized = 0;
    if (initialized) return;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        global_file_table[i].in_use = 0;
        global_file_table[i].refcount = 0;
    }
    initialized = 1;
}

// Garantiza que el FS esté inicializado
static int ensure_fs_initialized(void) {
    if (!fs_is_ready()) {
        if (fs_init() != 0) {
            printf("Error: no se pudo inicializar el FS\n");
            return -1;
        }
    }
    return 0;
}

// Busca una entrada libre en la tabla global
static int find_free_global_entry(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!global_file_table[i].in_use) return i;
    }
    return -1;
}

// Busca un fd libre en la tabla del proceso
static int find_free_fd(process_t *proc) {
    for (int i = 0; i < MAX_PROCESS_FD; i++) {
        if (!proc->fds[i].is_open) return i;
    }
    return -1;
}

// --- Abrir archivo ---
int file_open(process_t *proc, const char *filename, uint32_t flags) {
    if (!proc || !filename) return -1;
    if (ensure_fs_initialized() != 0) return -1;

    init_global_table();

    // Buscar o crear archivo
    uint32_t inode_num;
    if (fs_find_file(filename, &inode_num) != 0) {
        if (flags & O_CREAT) {
            if (fs_create_file(filename, FILE_TYPE_REGULAR) != 0) return -1;
            if (fs_find_file(filename, &inode_num) != 0) return -1;
        } else return -1;
    }

    // Buscar entrada libre en tabla global
    int gidx = find_free_global_entry();
    if (gidx < 0) return -1;

    // Inicializar entrada global
    inode_t *file_inode = fs_get_inode(inode_num);
    if (!file_inode) return -1;

    global_file_table[gidx].inode_num = inode_num;
    global_file_table[gidx].flags = flags;
    global_file_table[gidx].position = (flags & O_APPEND) ? file_inode->size : 0;
    global_file_table[gidx].refcount = 1;
    global_file_table[gidx].in_use = 1;

    // Buscar fd en el proceso
    int fd = find_free_fd(proc);
    if (fd < 0) {
        global_file_table[gidx].refcount = 0;
        global_file_table[gidx].in_use = 0;
        return -1;
    }

    proc->fds[fd].is_open = 1;
    proc->fds[fd].global_index = gidx;

    // O_TRUNC → truncar
    if (flags & O_TRUNC) {
        file_inode->size = 0;
        for (int i = 0; i < 12; i++) {
            if (file_inode->blocks[i]) {
                fs_free_block(file_inode->blocks[i]);
                file_inode->blocks[i] = 0;
            }
        }
        if (file_inode->indirect_block) {
            fs_free_block(file_inode->indirect_block);
            file_inode->indirect_block = 0;
        }
    }

    return fd;
}

// --- Cerrar archivo ---
int file_close(process_t *proc, int fd) {
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open) return -1;

    int gidx = proc->fds[fd].global_index;
    proc->fds[fd].is_open = 0;

    if (--global_file_table[gidx].refcount == 0) {
        global_file_table[gidx].in_use = 0;
    }

    return 0;
}

// --- Leer archivo ---
int file_read(process_t *proc, int fd, void *buffer, uint32_t count) {
    if (ensure_fs_initialized() != 0) return -1;
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open || !buffer) return -1;

    global_file_entry_t *entry = &global_file_table[proc->fds[fd].global_index];
    if (!(entry->flags & O_RDONLY) && !(entry->flags & O_RDWR)) return -1;

    inode_t *file_inode = fs_get_inode(entry->inode_num);
    if (!file_inode) return -1;

    uint32_t pos = entry->position;
    uint32_t file_size = file_inode->size;
    if (pos >= file_size) return 0; // EOF

    if (pos + count > file_size) count = file_size - pos;

    uint32_t bytes_read = 0;
    uint8_t *buf = (uint8_t *)buffer;

    while (bytes_read < count) {
        uint32_t block_index = pos / BLOCK_SIZE;
        uint32_t block_offset = pos % BLOCK_SIZE;
        uint32_t bytes_to_read = BLOCK_SIZE - block_offset;
        if (bytes_to_read > (count - bytes_read)) bytes_to_read = count - bytes_read;

        uint32_t block_num = (block_index < 12) ? file_inode->blocks[block_index] : 0;

        if (block_num == 0) {
            for (uint32_t i = 0; i < bytes_to_read; i++) buf[bytes_read + i] = 0;
        } else {
            uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE) + block_offset;
            for (uint32_t i = 0; i < bytes_to_read; i++) buf[bytes_read + i] = block_data[i];
        }

        bytes_read += bytes_to_read;
        pos += bytes_to_read;
    }

    entry->position = pos;
    return bytes_read;
}

// --- Escribir archivo ---
int file_write(process_t *proc, int fd, const void *buffer, uint32_t count) {
    if (ensure_fs_initialized() != 0) return -1;
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open || !buffer) return -1;

    global_file_entry_t *entry = &global_file_table[proc->fds[fd].global_index];
    if (!(entry->flags & O_WRONLY) && !(entry->flags & O_RDWR)) return -1;

    inode_t *file_inode = fs_get_inode(entry->inode_num);
    if (!file_inode) return -1;

    uint32_t pos = entry->position;
    uint32_t bytes_written = 0;
    const uint8_t *buf = (const uint8_t *)buffer;

    while (bytes_written < count) {
        uint32_t block_index = pos / BLOCK_SIZE;
        uint32_t block_offset = pos % BLOCK_SIZE;
        uint32_t bytes_to_write = BLOCK_SIZE - block_offset;
        if (bytes_to_write > (count - bytes_written)) bytes_to_write = count - bytes_written;

        uint32_t block_num = 0;
        if (block_index < 12) {
            block_num = file_inode->blocks[block_index];
            if (block_num == 0) {
                block_num = fs_allocate_block();
                if (block_num == 0) break;
                file_inode->blocks[block_index] = block_num;
                uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE);
                for (int i = 0; i < BLOCK_SIZE; i++) block_data[i] = 0;
            }
        }

        if (block_num == 0) break;

        uint8_t *block_data = fs_storage + (block_num * BLOCK_SIZE) + block_offset;
        for (uint32_t i = 0; i < bytes_to_write; i++) {
            block_data[i] = buf[bytes_written + i];
        }

        bytes_written += bytes_to_write;
        pos += bytes_to_write;
    }

    if (pos > file_inode->size) file_inode->size = pos;
    entry->position = pos;

    return bytes_written;
}

// --- Seek ---
int file_seek(process_t *proc, int fd, uint32_t offset, int whence) {
    if (ensure_fs_initialized() != 0) return -1;
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open) return -1;

    global_file_entry_t *entry = &global_file_table[proc->fds[fd].global_index];
    inode_t *file_inode = fs_get_inode(entry->inode_num);
    if (!file_inode) return -1;

    uint32_t new_pos;
    switch (whence) {
        case SEEK_SET: new_pos = offset; break;
        case SEEK_CUR: new_pos = entry->position + offset; break;
        case SEEK_END: new_pos = file_inode->size + offset; break;
        default: return -1;
    }

    entry->position = new_pos;
    return new_pos;
}

// --- Tell ---
int file_tell(process_t *proc, int fd) {
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open) return -1;
    return global_file_table[proc->fds[fd].global_index].position;
}

// --- EOF ---
int file_eof(process_t *proc, int fd) {
    if (!proc || fd < 0 || fd >= MAX_PROCESS_FD || !proc->fds[fd].is_open) return 1;
    global_file_entry_t *entry = &global_file_table[proc->fds[fd].global_index];
    inode_t *file_inode = fs_get_inode(entry->inode_num);
    if (!file_inode) return 1;
    return entry->position >= file_inode->size;
}
