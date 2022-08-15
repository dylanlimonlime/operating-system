#ifndef _PCB_H
#define _PCB_H

#include "types.h"
#include "terminal.h"

// constants used for file array in pcb
#define MIN_NUM_OF_FILES    2
#define MAX_NUM_OF_FILES    8
#define MAX_ARGS			128	
#define MAX_PROCESSES		6

// structure for file entry
typedef struct fentry {
    int32_t (*operations_table[4])();
	int32_t inode;
	int32_t file_position;
	int32_t flags;
} fentry_t;

// structure for process control block
typedef struct pcb_t {
    uint8_t args[MAX_ARGS]; // command args
    uint8_t process_id; // process number
    uint32_t esp;       // esp and ebp for execute/halt
    uint32_t ebp;
    uint32_t scheduler_ebp; // ebp for scheduler
    struct pcb_t* parent_pcb; // ptr to parent pcb
    struct pcb_t* child_pcb; // ptr to child
    fentry_t file_array[MAX_NUM_OF_FILES]; // Files
} pcb_t;

// functions for pcb
pcb_t* find_pcb();
void set_pcb(pcb_t* new_pcb);
pcb_t* pcb_init(uint8_t* command_str, uint8_t* arg,	uint8_t process_number);
void add_reg(uint32_t ebp, uint32_t esp);
void save_scheduler_reg(uint32_t ebp);

#endif
