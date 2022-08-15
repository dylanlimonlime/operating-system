#include "paging.h" 
#include "types.h"
#include "x86_desc.h"
#include "syscall.h"

// array for page directory entries
static unsigned long page_directory_entries[NUM_OF_ENTRIES] __attribute__((aligned(TOTAL_SIZE)));

// array for page table entries
static unsigned long page_table_entries[NUM_OF_ENTRIES] __attribute__((aligned(TOTAL_SIZE)));

static unsigned long video_mem_page_table[NUM_OF_ENTRIES] __attribute__((aligned(TOTAL_SIZE)));

/* 
   FUNCTION:    paging_init()
   DESCRIPTION: This function turns on paging and ensures that kernel continues
                to work. First, all entries are marked as non-present in both
                the page directory and table. Then, entries for the kernel and 
                video memory are created in both and pages are marked as 
                present. Finally, assembly code is used to correctly set 
                registers cr0, cr3, and cr4 in order to actually enable paging.
   INPUTS:      None
   OUTPUTS:     None 
*/
void paging_init() {
    
    int i; // variable used for iteration through number of entries (1024)
    for(i = START; i < NUM_OF_ENTRIES; i++) {
        
        // intialize page directory with non-present entries
        page_directory_entries[i] = (FLAG_RW);

        // initialize page table with non-present entries
        page_table_entries[i] = (i * TOTAL_SIZE) | (FLAG_RW);

    }

    // add first page directory entry for page table with 
    // pages are present and can be read and written-into
    page_directory_entries[0] = ((unsigned long)page_table_entries) | (FLAG_P) | (FLAG_RW);
    
    // Initialize video memory page table
    page_directory_entries[VIDMEM_DIR_IDX] = ((unsigned long)video_mem_page_table) | FLAG_P | FLAG_RW | FLAG_US;

    // add page table entry for video memory
    // pages are present and can be read and written-into
    page_table_entries[((VIDEO_MEMORY) >> TABLE_INDEX_SHIFT)] = (VIDEO_MEMORY) | (FLAG_P) | (FLAG_RW);
    
    // Add video memory bakup pages for multiple terminals
    page_table_entries[VMEM_BAK_ONE_IDX] = VMEM_BAK_ONE_ADDR | FLAG_P | FLAG_RW | FLAG_US;
    page_table_entries[VMEM_BAK_TWO_IDX] = VMEM_BAK_TWO_ADDR | FLAG_P | FLAG_RW | FLAG_US;
    page_table_entries[VMEM_BAK_THREE_IDX] = VMEM_BAK_THREE_ADDR | FLAG_P | FLAG_RW | FLAG_US;



    // add second page directory entry for kernel
    // pages are present, size is 4MB, and can be read and written-into
    page_directory_entries[1] = (KERNEL_ADDRESS) | (FLAG_P) | (FLAG_RW) | (FLAG_PS);

    // assembly language to turn on paging
    asm volatile (   
        "movl   %0, %%eax;"                      // eax <- 0
        "movl   %%eax, %%cr3;"                   // cr3 <- eax    
        "movl   %%cr4, %%eax;"                   // eax <- cr4
        "orl    $0x00000010, %%eax;"             // OR eax with 0x00000010
        "movl   %%eax, %%cr4;"                   // cr4 <- eax
        "movl   %%cr0, %%eax;"                   // eax <- cr0
        "orl    $0x80000001, %%eax;"             // OR eax with 0x80000001
        "movl   %%eax, %%cr0;"                   // cr0 <- eax
        :
        : "r"(page_directory_entries)            // page directory input
        : "eax"                                  // eax gets clobbered during data movement
    );
}

/* get_page_directory
 * INPUTS: dir_idx - Index of requested entry in page directory
 * RETURNS: pointer to requested entry
 * SIDE EFFECTS: None
 */
uint32_t* get_page_directory(uint32_t dir_idx) {
    return (uint32_t*)&(page_directory_entries[dir_idx]);
}

/* get_page_table_entry
 * INPUTS: dir_idx - Index of requested entry in kernel page table
 * RETURNS: pointer to requested entry
 * SIDE EFFECTS: None
 */
uint32_t* get_table_entry(uint32_t table_idx) {
    return (uint32_t*)&(page_table_entries[table_idx]);
}

/* get_vidmem_entry
 * INPUTS: dir_idx - Index of requested entry in vidmem table
 * RETURNS: pointer to requested entry
 * SIDE EFFECTS: None
 */
uint32_t* get_vidmem_entry(uint32_t table_idx) {
    return (uint32_t*)&(video_mem_page_table[table_idx]);
}
