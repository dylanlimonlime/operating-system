/* filesys.h
 * Header file for filesystem
 */

#ifndef _FILESYS_H
#define _FILESYS_H

#include "paging.h"
#include "lib.h"
#include "types.h"
#include "pcb.h"
#include "syscall.h"


/* constants */
#define BLOCK_SIZE          4096     //Each block is 4kB
#define FNAME_LEN           32       //up to 32 characters
#define DENTRY_RESERVED     24       //24B reserved
#define BOOT_BLOCK_RESERVED 52
#define DENTRY_SIZE         64       //64B
#define MAX_DENTRIES        63
#define DATA_BLOCK_SIZE     1023

/* testing constants */
#define ARBITRARY_BUFFER_SIZE 33
#define MAGIC_NUMBER_OFFSET   100
#define BIG_BUF_SIZE          100000

/* structs */
typedef struct {
    uint8_t fname[FNAME_LEN];               //32B file name
    uint32_t ftype;                         //4B file type
    uint32_t inode_num;                     //4B inode #
    uint8_t reserved[DENTRY_RESERVED];      //24B reserved
} dentry_t;

typedef struct {
    uint32_t dentries_count;                  //4B # of dir. entries
    uint32_t inodes_count;                    //4B # of inodies
    uint32_t data_blocks_count;               //4B # of data blocks
    uint8_t reserved[BOOT_BLOCK_RESERVED];    //52B reserved
    dentry_t dentries[MAX_DENTRIES];          //directory entires, max 63
} boot_block_t;

typedef struct {
    uint32_t length;                          //4B length of data block
    uint32_t data_block[DATA_BLOCK_SIZE];     //Max 4kB, 4B blocks, first 4B for size
} inode_t;

typedef struct {
    uint8_t data[BLOCK_SIZE];                 //4kB data
} data_block_t;

/* file system initialization */
void filesys_init(uint32_t multiboot_module_addr);

/* helper functions */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
inode_t get_inode(uint32_t inode_idx);

/* directory functions */
extern int32_t directory_open(const uint8_t* filename);
extern int32_t directory_close(int32_t fd);
extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

/* file functions */
extern int32_t file_open(const uint8_t* filename);
extern int32_t file_close(int32_t fd);
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
