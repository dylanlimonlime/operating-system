#ifndef _PAGING_H
#define _PAGING_H

#include "types.h"

/* ------------------------------- CONSTANTS ------------------------------- */
#define KERNEL_ADDRESS        0x400000 // address at which kernel is loaded
#define VIDEO_MEMORY          0xB8000  // address for video memory
#define NUM_OF_ENTRIES        1024     // total number of entries in directory/table
#define TOTAL_SIZE            4096     // total size of directory/table
#define TABLE_INDEX_SHIFT     12       // page table index offset
#define DIRECTORY_INDEX_SHIFT 22       // directory table index offset
#define TEN_INDEX_SHIFT       0x3FF    // 10 bits of shifting
#define VIDEO_MEM_INSIDE      0xB800A  // location inside video memory for testing purposes
#define KERNEL_MEM_INSIDE     0x400002 // location inside kernel memory for testing
#define TABLE_INSIDE          4098
#define INVALID_ADDR          0x0
#define VIDMEM_DIR_IDX        33

#define VMEM_BAK_ONE_IDX      0xB9
#define VMEM_BAK_TWO_IDX      0xBA
#define VMEM_BAK_THREE_IDX    0xBB

#define VMEM_BAK_ONE_ADDR     0xB9000
#define VMEM_BAK_TWO_ADDR     0xBA000
#define VMEM_BAK_THREE_ADDR   0xBB000


/* ------------------------------------------------------------------------- */

/* --------------------------- SECTION FOR FLAGS --------------------------- */
/*
   FLAG_P (Present)
   SET:   Page is in physical memory and address translation can be carried out
   CLEAR: Page is not in memory, so page-fault exception can arise if processor 
          attempts to access the page
*/
#define FLAG_P   0x1  

/*
   FLAG_RW (Read/Write)
   SET:   Page can be read and written-into
   CLEAR: Page is read-only
*/
#define FLAG_RW  0x2

/*
   FLAG_US (User/Supervisor)
   SET:   Page is assigned user privilege level
   CLEAR: Page is assigned supervisor privilege level
*/
#define FLAG_US  0x4      

/*
   FLAG_PWT (Page-Level Write-Through)
   SET:   Write-through caching is enabled for the associated page or page 
          table
   CLEAR: Write-back caching is enabled for the associated page or page table
*/
#define FLAG_PWT 0x8      

/*
   FLAG_PCD (Page-Level Cache Disable)
   SET:   Caching of the associated page or page table is prevented
   CLEAR: Page or page table can be cached
*/
#define FLAG_PCD 0x10     

/*
   FLAG_A (Accessed)
   SET:   First time page has been accessed
   CLEAR: Page has not been accessed after being intially loaded into physical 
          memory
*/
#define FLAG_A   0x20     

/*
   FLAG_D (Dirty)
   SET:   First time page has been accessed for a write operation
   CLEAR: Page has not been accessed after being intially loaded into physical
          memory
*/
#define FLAG_D   0x40     

/*
   FLAG_PS (Page Size)
   SET:   Page size is 4MB and page-directory entry points to a page
   CLEAR: Page size is 4KB and page-directory entry points to a page table, 
          where all pages associated with page table will be of size 4KB
*/
#define FLAG_PS  0x80

/*
   FLAG_G (Global)
   SET:   Indicates global page
   CLEAR: Page is not global
*/
#define FLAG_G   0x100
/* ------------------------------------------------------------------------- */

// function to intialize paging
void paging_init();

// Returns a pointer to the page directory entry at the specified index
extern uint32_t* get_page_directory(uint32_t dir_idx);

// Returns a pointer to the page table entry at the specified index at the 
// table in directory index 0
extern uint32_t* get_table_entry(uint32_t table_idx);

// Returns a pointer to the page table entry at the specified index at the user
// video memory page
extern uint32_t* get_vidmem_entry(uint32_t table_idx);

#endif 
