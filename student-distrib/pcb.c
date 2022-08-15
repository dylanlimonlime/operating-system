
#include "pcb.h"

// Address of current PCB
pcb_t* curr_addr = NULL;

int32_t (*stdin_operations_table[NUM_OF_OPERATIONS])()={terminal_read, terminal_fail, terminal_open, terminal_close};
int32_t (*stdout_operations_table[NUM_OF_OPERATIONS])()={terminal_fail, terminal_write, terminal_open, terminal_close};

/*
FUNCTION NAME: find_pcb
DESCRIPTION:   returns pcb address for the current process
INPUTS:        none
OUTPUTS:       pcb address
SIDE EFFECTS:  none
*/
pcb_t* find_pcb() {
    return curr_addr;
}

/*
 * set_pcb
 * DESCRIPTION: Changes the current pcb
 * INPUTS:  new_pcb - pointer to the new pcb
 * OUTPUTS: NONE
 * SIDE EFFECTS: curr_addr gets overwritten
 */
void set_pcb(pcb_t* new_pcb) {
    curr_addr = new_pcb;
}

/*
FUNCTION NAME: pcb_init
DESCRIPTION:   creates a pcb using the given command and process number and initializes file array
INPUTS:        command_str
               arg (for cp4)
               process_number
OUTPUTS:       pcb address
SIDE EFFECTS:  creates pcb in kernel
*/
pcb_t* pcb_init(uint8_t* command_str, uint8_t* arg,	uint8_t process_number){
    
    int i;
    if(process_number >= MAX_PROCESSES) { return NULL; }
    
    // Get the pointer for the new PCB
    pcb_t* pcb = (pcb_t*)(PHYS_PAGE_START - ((process_number + 1) * PROG_STACK_SIZE));
    pcb->parent_pcb = curr_addr;
    pcb->process_id = process_number;
    if (curr_addr != NULL) { curr_addr->child_pcb = pcb; }
   
    // Clear all the fields that won't be set now
    pcb->child_pcb = NULL;
    pcb->esp = 0x00000000;
    pcb->ebp = 0x00000000;
    pcb->scheduler_ebp = 0x00000000;

    // Clear all PCB files
    for(i = START; i < MAX_FILES; i++){
        pcb->file_array[i].inode = NONE;
        pcb->file_array[i].file_position = START;
        pcb->file_array[i].flags = AVAILABLE;
    }

    // initialize pcb files for stdin and stdout
    for(i = START; i < NUM_OF_OPERATIONS; i++) {
        pcb->file_array[0].operations_table[i] = stdin_operations_table[i];
        pcb->file_array[1].operations_table[i] = stdout_operations_table[i];
    }
    pcb->file_array[0].flags= OCCUPIED;
    pcb->file_array[1].flags = OCCUPIED;

    return pcb;

}

/* void add_reg
 * DESCRIPTION: Sets the current PCB's ebp and esp fields to the arguments
 * INPUTS:  ebp - The value of the ebp
 *          esp - The value of the esp
 * OUTPUTS: None
 * SIDE EFFECTS: PCB's current ebp and esp values are cleared
 */
void add_reg(uint32_t ebp, uint32_t esp) {
    curr_addr->ebp = ebp;
    curr_addr->esp = esp;
}

/*
 * save_scheduler_reg
 * DESCRIPTION: Saves the EBP for the scheduler
 * INPUTS:  ebp - The value of the ebp
 * OUTPUTS: NONE
 * SIDE EFFECTS: The previous value of scheduler_ebp gets overwritten
 */

void save_scheduler_reg(uint32_t ebp) {
    curr_addr->scheduler_ebp = ebp;
}
