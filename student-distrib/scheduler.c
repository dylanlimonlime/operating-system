#ifndef _SCHEDULER_C
#define _SCHEDULER_C

#include "scheduler.h"
char* keys[3] = {keyboard_buf_0, keyboard_buf_1, keyboard_buf_2};
int8_t terminal_colors[3] = {BLACK + (WHITE << BACKGROUND), LIGHT_RED + (DARK_GRAY << BACKGROUND), LIGHT_GREEN + (LIGHT_BLUE << BACKGROUND)};

/*
 * scheduler
 * DESCRIPTION: Switches the current process to the next one
 * INPUTS:  NONE
 * OUTPUTS: NONE
 * SIDE EFFECTS: NONE
 */
void scheduler() {
    // Get the process ID and terminal number of the next terminal
    uint8_t next_term = (cur_scheduled_terminal + 1) % 3;
    int8_t next_process_num = terminal_process_nums[next_term];
    
    // Check if this is the first time running the scheduler
    if (next_term != 1 || next_process_num != NOT_ASSIGNED) {
        scheduler_save_reg(); // Save the ebp of the scheduler for when we come back
    }
    // Check if this is the first time we're switching to the next process
    if (next_process_num == NOT_ASSIGNED) {
        terminal_process_nums[next_term] = next_available_process();
        set_pcb(NULL);
        const char* shell = "shell";
	cur_scheduled_terminal = next_term;
	//sets the keyboard buf to scheduled one
        keyboard_buf = keys[cur_scheduled_terminal];
        VIDEO_MEM_OFFSET = cur_terminal == cur_scheduled_terminal ? 0 : (cur_scheduled_terminal + 1)*FOUR_KB;
	//Calculates the VIDEO_MEM_OFFSET based off the cur scheduled terminal unless its the same as the cur_terminal
        ARRTIB = terminal_colors[cur_scheduled_terminal];
    
        send_eoi(0);
        execute((uint8_t*)shell);
        return;
    }
    // If the terminal is running a process, ensure that we switch to that PCB
    pcb_t* next_pcb = (pcb_t*)(PHYS_PAGE_START - ((next_process_num + 1) * PROG_STACK_SIZE));
    while (next_pcb->child_pcb != NULL) { next_pcb = next_pcb->child_pcb; }
    next_process_num = next_pcb->process_id;
   
    // Change the PCB
    set_pcb(next_pcb);
    
    // Setup paging for next process
    uint32_t* program_page = get_page_directory(PROG_PAGE_IDX);
    uint32_t prog_mem = PHYS_PAGE_START + (PROGRAM_SIZE * next_process_num);
    *program_page = 0x00000000;
    *program_page = prog_mem | FLAG_P | FLAG_RW | FLAG_US | FLAG_PS;
    
    // Flush TLB
    asm volatile (
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        :
        :
        :"%eax"
    );
    
    // Set the current terminal to the next one
    cur_scheduled_terminal = next_term;
    keyboard_buf = keys[cur_scheduled_terminal]; //sets the keyboard_buf to the scheduled one
    VIDEO_MEM_OFFSET = cur_terminal == cur_scheduled_terminal ? 0 : (cur_scheduled_terminal + 1)*FOUR_KB; //calculates the VIDEO_MEM_OFFSET based off the cure_scheduled terminal in relation to the cur_terminal
    if (cur_scheduled_terminal != cur_terminal) {CURSOR = 0;} //sets the cursor on if the scheduled terminal is the current terminal
    else {CURSOR = 1;}
    ARRTIB = terminal_colors[cur_scheduled_terminal]; //changes text_colour to match the terminal

    // ss0 will contain the location of the Kernel Data Segment
    tss.ss0 =  KERNEL_DS;
    // esp0 will point to the new stack pointer
    tss.esp0 = PHYS_PAGE_START - (next_process_num * PROG_STACK_SIZE) - LONG;
    
    // Send an eoi to the PIT so that another interrupt can come in
    send_eoi(0);
    // Call assembly function to do final switch
    scheduler_switch_process(next_pcb->scheduler_ebp);
    
    return; 
}

#endif
