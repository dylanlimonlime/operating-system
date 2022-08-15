
#include "multi_term.h"

/* swap_terminal(uint8_t new_terminal)
 * DESCRIPTION: This function is called whenever a shell switch is called
 * with an ALT + FN. 
 * INPUT: The index of the new terminal to be switched to
 * OUTPUT: Returns 0 for success and -1 for fail
 * SIDE EFFECTS: Switches the video memory to the video memory of the new_terminal
 */
int8_t swap_terminal(uint8_t new_terminal) {
    // Make sure input is valid
    if (new_terminal == cur_terminal || new_terminal >= MAX_TERMINALS) {
        return -1;
    }
    // Save old video memory page into backup
    memcpy((uint8_t*)(VMEM_BAK_BASE_ADDR + (cur_terminal * FOUR_KB)), (uint8_t*)VMEM, FOUR_KB);
    
    // Move backup video memory into main page
    memcpy((uint8_t*)VMEM, (uint8_t*)(VMEM_BAK_BASE_ADDR + (new_terminal * FOUR_KB)), FOUR_KB); 
  
    pcb_t* cur_term_pcb = (pcb_t*)(PHYS_PAGE_START - ((terminal_process_nums[cur_terminal] + 1) * PROG_STACK_SIZE));
    pcb_t* new_term_pcb = (pcb_t*)(PHYS_PAGE_START - ((terminal_process_nums[new_terminal] + 1) * PROG_STACK_SIZE));
    // Switches Vidmap video memory to the memory of where that terminal is stored
    cur_term_pcb = cur_term_pcb->child_pcb;
    while (cur_term_pcb != NULL) {
        uint32_t* vidmem_entry = get_vidmem_entry(cur_term_pcb->process_id);
        *vidmem_entry = (*vidmem_entry & 0xFF) | (VMEM_BAK_BASE_ADDR + (cur_terminal * FOUR_KB));
        cur_term_pcb = cur_term_pcb->child_pcb;
    }
    
    new_term_pcb = new_term_pcb->child_pcb;
    while (new_term_pcb != NULL) {
        uint32_t* vidmem_entry = get_vidmem_entry(new_term_pcb->process_id);
        *vidmem_entry = (*vidmem_entry & 0xFF) | VMEM;
        new_term_pcb = new_term_pcb->child_pcb;
    }

    //switches flashing cursor to new location of screenx and screeny
    flashy_set(NUM_COLS*sscreeny[new_terminal] + sscreenx[new_terminal]);

    //switches keyboard buffer to new_terminal keyboard buffer
    if (new_terminal == 0) {cur_keyboard = keyboard_buf_0; text_colour(BLACK, WHITE);}
    else if (new_terminal == 1) {cur_keyboard = keyboard_buf_1; text_colour(LIGHT_RED,DARK_GRAY);}
    else {cur_keyboard = keyboard_buf_2; text_colour(LIGHT_GREEN,LIGHT_BLUE);}

    //makes the new_terminal the current terminal
    cur_terminal = new_terminal;
    if (cur_scheduled_terminal != cur_terminal) {CURSOR = 0;} //if the new terminal isnt the scheduled one,dont use the cursor
    else {CURSOR = 1;} //cursor goes on if at current scheduled terminal and current terminal
    VIDEO_MEM_OFFSET = cur_terminal == cur_scheduled_terminal ? 0 : (cur_scheduled_terminal + 1)*FOUR_KB; //one is added since cur_scheduled terminal is an index
    return 0;

}
