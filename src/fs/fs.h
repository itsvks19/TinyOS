#pragma once

#include <stdint.h>

#define FS_MAGIC        "NANOFS2"   /* 7 bytes + NUL                       */
#define FS_VERSION      2
#define FS_MAX_INODES   80          /* 8 sectors × 10 inodes per sector     */
#define FS_INODE_TABLE_START  1     /* first sector of inode table          */
#define FS_INODE_TABLE_SECTORS 8
#define FS_DATA_START   9           /* first data sector                    */
#define FS_MAX_BLOCKS   4           /* blocks per file (4 × 512 = 2 KiB)   */
#define FS_NAME_MAX     28          /* max filename length incl. NUL        */


typedef struct __attribute__((packed)) {
    char     name[FS_NAME_MAX];
    uint32_t parent;
    uint32_t blocks[FS_MAX_BLOCKS];
    uint32_t size;
    uint8_t  is_directory;              
    
    uint8_t  _pad[1];
} fs_inode_t;

#undef  FS_MAX_INODES
#define FS_MAX_INODES          72 
#undef  FS_INODE_TABLE_SECTORS
#define FS_INODE_TABLE_SECTORS  8

typedef struct __attribute__((packed)) {
    char     magic[8];
    uint32_t version;
    uint32_t total_sectors;
    uint32_t inode_count;
} fs_superblock_t;

void fs_format(void);
int  fs_mount(void);

int  fs_create(const char *name); 
int  fs_write(const char *name, const char *data);
int  fs_append(const char *name, const char *data);
int  fs_read(const char *name, char *buffer, uint32_t buf_size);
int  fs_delete(const char *name);
int  fs_rename(const char *old_name, const char *new_name);

/* Directory ops */
int  fs_mkdir(const char *name);
int  fs_change_dir(const char *name);
int  fs_list(fs_inode_t *out, int max_out);

const char *fs_get_pwd(void);
int  fs_stat(const char *name, fs_inode_t *out); 