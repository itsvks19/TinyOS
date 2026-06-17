#include "fs.h"
#include "../drivers/ata/ata.h"

static uint32_t current_dir = 0;

static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static void str_copy(char *dst, const char *src, int max) {
    int i;
    for (i = 0; i < max - 1 && src[i]; i++)
        dst[i] = src[i];
    dst[i] = '\0';
}

static int str_len(const char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

#define INODES_PER_SECTOR  (512 / sizeof(fs_inode_t))

static void inode_read(uint32_t id, fs_inode_t *out) {
    if (id == 0 || id > FS_MAX_INODES) {
        for (int i = 0; i < (int)sizeof(fs_inode_t); i++)
            ((uint8_t *)out)[i] = 0;
        return;
    }
    uint32_t idx    = id - 1;
    uint32_t sector = FS_INODE_TABLE_START + idx / INODES_PER_SECTOR;
    uint32_t slot   = idx % INODES_PER_SECTOR;

    uint8_t buf[512];
    ata_read_sector(sector, buf);
    fs_inode_t *table = (fs_inode_t *)buf;
    *out = table[slot];
}

static void inode_write(uint32_t id, const fs_inode_t *in) {
    if (id == 0 || id > FS_MAX_INODES) return;
    uint32_t idx    = id - 1;
    uint32_t sector = FS_INODE_TABLE_START + idx / INODES_PER_SECTOR;
    uint32_t slot   = idx % INODES_PER_SECTOR;

    uint8_t buf[512];
    ata_read_sector(sector, buf);
    fs_inode_t *table = (fs_inode_t *)buf;
    table[slot] = *in;
    ata_write_sector(sector, buf);
}

static uint32_t inode_alloc(void) {
    for (uint32_t id = 1; id <= FS_MAX_INODES; id++) {
        fs_inode_t node;
        inode_read(id, &node);
        if (node.name[0] == '\0')
            return id;
    }
    return 0;
}

static uint32_t block_alloc(void) {
    uint32_t highest = FS_DATA_START - 1;
    for (uint32_t id = 1; id <= FS_MAX_INODES; id++) {
        fs_inode_t node;
        inode_read(id, &node);
        if (node.name[0] == '\0') continue;
        for (int b = 0; b < FS_MAX_BLOCKS; b++) {
            if (node.blocks[b] > highest)
                highest = node.blocks[b];
        }
    }
    return highest + 1;
}

static uint32_t lookup(const char *name, uint32_t parent_dir) {
    for (uint32_t id = 1; id <= FS_MAX_INODES; id++) {
        fs_inode_t node;
        inode_read(id, &node);
        if (node.name[0] == '\0') continue;
        if (node.parent != parent_dir) continue;
        if (str_eq(node.name, name)) return id;
    }
    return 0;
}

void fs_format(void) {
    uint8_t sector[512] = {0};
    fs_superblock_t *sb = (fs_superblock_t *)sector;
    str_copy(sb->magic, FS_MAGIC, sizeof(sb->magic));
    sb->version       = FS_VERSION;
    sb->total_sectors = 131072;
    sb->inode_count   = 0;
    ata_write_sector(0, sector);

    uint8_t empty[512] = {0};
    for (int s = 0; s < FS_INODE_TABLE_SECTORS; s++)
        ata_write_sector(FS_INODE_TABLE_START + s, empty);
}

int fs_mount(void) {
    uint8_t sector[512];
    if (ata_read_sector(0, sector) != 0) return 0;
    fs_superblock_t *sb = (fs_superblock_t *)sector;
    return str_eq(sb->magic, FS_MAGIC);
}

int fs_create(const char *name) {
    if (lookup(name, current_dir)) return -1;

    uint32_t id = inode_alloc();
    if (id == 0) return -2;

    fs_inode_t node = {0};
    str_copy(node.name, name, FS_NAME_MAX);
    node.parent       = current_dir;
    node.is_directory = 0;
    node.size         = 0;
    node.blocks[0]    = block_alloc();

    inode_write(id, &node);
    return 0;
}

int fs_write(const char *name, const char *data) {
    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;

    fs_inode_t node;
    inode_read(id, &node);
    if (node.is_directory) return -2;

    int len = str_len(data);
    int max = FS_MAX_BLOCKS * 511; 
    if (len > max) len = max;

    int blocks_needed = (len + 510) / 511;
    if (blocks_needed < 1) blocks_needed = 1;

    for (int b = 0; b < blocks_needed; b++) {
        if (node.blocks[b] == 0)
            node.blocks[b] = block_alloc();
    }
    int written = 0;
    for (int b = 0; b < blocks_needed && written < len; b++) {
        uint8_t buf[512] = {0};
        int chunk = len - written;
        if (chunk > 511) chunk = 511;
        for (int j = 0; j < chunk; j++)
            buf[j] = data[written + j];
        ata_write_sector(node.blocks[b], buf);
        written += chunk;
    }

    node.size = len;
    inode_write(id, &node);
    return 0;
}

int fs_append(const char *name, const char *data) {
    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;

    fs_inode_t node;
    inode_read(id, &node);
    if (node.is_directory) return -2;

    int extra    = str_len(data);
    int new_size = node.size + extra;
    int max      = FS_MAX_BLOCKS * 511;
    if (new_size > max) new_size = max;
    extra = new_size - (int)node.size;
    if (extra <= 0) return 0;

    char scratch[FS_MAX_BLOCKS * 512];
    int  pos = 0;
    for (int b = 0; b < FS_MAX_BLOCKS && pos < (int)node.size; b++) {
        if (node.blocks[b] == 0) break;
        uint8_t buf[512];
        ata_read_sector(node.blocks[b], buf);
        int chunk = node.size - pos;
        if (chunk > 511) chunk = 511;
        for (int j = 0; j < chunk; j++)
            scratch[pos + j] = buf[j];
        pos += chunk;
    }
    /* Append new data */
    for (int i = 0; i < extra; i++)
        scratch[node.size + i] = data[i];
    inode_write(id, &node);
    return fs_write(name, scratch);
}

int fs_read(const char *name, char *buffer, uint32_t buf_size) {
    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;

    fs_inode_t node;
    inode_read(id, &node);
    if (node.is_directory) return -2;

    uint32_t to_read = node.size;
    if (to_read >= buf_size) to_read = buf_size - 1;

    uint32_t pos = 0;
    for (int b = 0; b < FS_MAX_BLOCKS && pos < to_read; b++) {
        if (node.blocks[b] == 0) break;
        uint8_t buf[512];
        ata_read_sector(node.blocks[b], buf);
        uint32_t chunk = to_read - pos;
        if (chunk > 511) chunk = 511;
        for (uint32_t j = 0; j < chunk; j++)
            buffer[pos + j] = buf[j];
        pos += chunk;
    }
    buffer[pos] = '\0';
    return (int)pos;
}

int fs_delete(const char *name) {
    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;

    fs_inode_t node;
    inode_read(id, &node);
    if (node.is_directory) return -2;

    fs_inode_t empty = {0};
    inode_write(id, &empty);
    return 0;
}

int fs_rename(const char *old_name, const char *new_name) {
    uint32_t id = lookup(old_name, current_dir);
    if (id == 0) return -1;
    if (lookup(new_name, current_dir)) return -2;

    fs_inode_t node;
    inode_read(id, &node);
    str_copy(node.name, new_name, FS_NAME_MAX);
    inode_write(id, &node);
    return 0;
}

int fs_mkdir(const char *name) {
    if (lookup(name, current_dir)) return -1;

    uint32_t id = inode_alloc();
    if (id == 0) return -2;

    fs_inode_t node = {0};
    str_copy(node.name, name, FS_NAME_MAX);
    node.parent       = current_dir;
    node.is_directory = 1;
    node.size         = 0;

    inode_write(id, &node);
    return 0;
}

int fs_change_dir(const char *name) {
    /* cd / */
    if (name[0] == '/' && name[1] == '\0') {
        current_dir = 0;
        return 0;
    }

    if (name[0] == '.' && name[1] == '.' && name[2] == '\0') {
        if (current_dir != 0) {
            fs_inode_t node;
            inode_read(current_dir, &node);
            current_dir = node.parent;
        }
        return 0;
    }

    if (name[0] == '.' && name[1] == '\0')
        return 0;

    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;

    fs_inode_t node;
    inode_read(id, &node);
    if (!node.is_directory) return -2;

    current_dir = id;
    return 0;
}

int fs_list(fs_inode_t *out, int max_out) {
    int count = 0;
    for (uint32_t id = 1; id <= FS_MAX_INODES && count < max_out; id++) {
        fs_inode_t node;
        inode_read(id, &node);
        if (node.name[0] == '\0') continue;
        if (node.parent != current_dir) continue;
        out[count++] = node;
    }
    return count;
}

int fs_stat(const char *name, fs_inode_t *out) {
    uint32_t id = lookup(name, current_dir);
    if (id == 0) return -1;
    inode_read(id, out);
    return 0;
}

const char *fs_get_pwd(void) {
    static char path[256];
    static char tmp[256];

    if (current_dir == 0) return "/";

    int pos = sizeof(tmp) - 1;
    tmp[pos] = '\0';

    uint32_t dir = current_dir;
    while (dir != 0) {
        fs_inode_t node;
        inode_read(dir, &node);
        int len = str_len(node.name);
        for (int i = len - 1; i >= 0; i--)
            tmp[--pos] = node.name[i];
        tmp[--pos] = '/';
        dir = node.parent;
    }

    str_copy(path, &tmp[pos], sizeof(path));
    return path;
}