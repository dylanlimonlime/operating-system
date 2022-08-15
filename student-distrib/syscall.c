#include "syscall.h"

#define FD_MAX 7

// Status of all processes
uint8_t avail_processes[MAX_PROCESSES] = {0, 0, 0, 0, 0, 0};


// Operations tables for PCB
int32_t (*rtc_operations_table[NUM_OF_OPERATIONS])() = {rtc_read, rtc_write, rtc_open, rtc_close};
int32_t (*directory_operations_table[NUM_OF_OPERATIONS])() = {directory_read, directory_write, directory_open, directory_close};
int32_t (*file_operations_table[NUM_OF_OPERATIONS])() = {file_read, file_write, file_open, file_close};

/*
FUNCTION NAME: halt
DESCRIPTION:   halt system and return control back to parent process
INPUTS:        status - flag of process
OUTPUTS:       returns 0 upon success or -1 upon failure
SIDE EFFECTS:  system is halted
*/
int32_t halt(uint8_t status) {
    
    // clear interrupts
    cli();
    
    // find current process
    pcb_t* current_pcb = find_pcb();

    // get parent process pointer to current process
    pcb_t* parent_pcb = current_pcb->parent_pcb;
    current_pcb->parent_pcb = NULL;

    // restarts shell if halt is called in the initial shell execution
    if (parent_pcb == NULL) {
        // frees up available process
        avail_processes[current_pcb->process_id] = AVAILABLE; 
        set_pcb((pcb_t*)0x0);
        execute((uint8_t*)"shell");

        // returns -1 for failure
        return FAILURE; 
    }

    parent_pcb->child_pcb = NULL; 
    
    // free up current pcb's process in array of available processes
    avail_processes[(uint8_t)current_pcb->process_id] = AVAILABLE;

    // close any files that may be open
    int32_t i;
    for(i = START; i < MAX_NUM_OF_FILES; i++) {
        if(current_pcb->file_array[i].flags == OCCUPIED) {close(i);}
    }
    
    current_pcb->args[0] = '\0';

    // set certain pcb fields to appropriate "unused" values
    set_pcb(parent_pcb);

    // reverse program paging
    uint32_t* program_page = get_page_directory(PROG_PAGE_IDX);
    uint32_t prog_mem = PHYS_PAGE_START + (PROGRAM_SIZE*parent_pcb->process_id);
    *program_page = 0x00000000; // Clear the page just in case
    *program_page = prog_mem | FLAG_P | FLAG_RW | FLAG_US | FLAG_PS;
  
    *get_vidmem_entry(current_pcb->process_id) = 0x00000000;

    // flush tlb
    asm volatile (
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        :
        :
        : "%eax"
    );

    // set ss0 and esp0 in tss
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PHYS_PAGE_START - (parent_pcb->process_id * PROG_STACK_SIZE) - LONG;

    // restore interrupts
    sti();
    
    //returns from halt with assembly
    halt_return(status & 0xFF, current_pcb->ebp, current_pcb->esp);

    return SUCCESS;
    
}

/*
 * int32_t execute(const uint8_t* command)
 * DESCRIPTION: This function takes a command and executes 
 *              the program specified by the command
 * INPUTS: command = the file system command to be executed
 * OUTPUTS: -1 for failure, 256 for exception, success for anything else
 * SIDE EFFECTS:  * executes a program based off the command
 */
int32_t execute(const uint8_t* command) {
    if (command == NULL) { return FAILURE; } //returns -1 for incorrect commands
    else if (command[0] == '\0') { return FAILURE; }
    
    // Check if there are open processes
    uint8_t process_num = 0;
    for (process_num = 0; process_num < MAX_PROCESSES; process_num++) {
        if (avail_processes[process_num] == 0) { break; } 
        // If we've found no available process, return failure
        else if (process_num == MAX_PROCESSES-1) { return FAILURE; }
    }
    cli();
    
    dentry_t dentry_ref;
    dentry_t* dentry = &dentry_ref;
    // Parse command
    uint8_t command_str[FNAME_LEN];
    int i;
    int spaces = 0;
    while (command[spaces] == ' ') { spaces++; } //removes inital spaces
    for (i = 0; i < FNAME_LEN; i++) { //parses command from input
        if (command[i + spaces] != ' ' && command[i + spaces] != '\n') {
            command_str[i] = command[i + spaces];
        }
        else {
            command_str[i] = '\0';
            break;
        }
    }


    // Create the PCB
    pcb_t* new_pcb = pcb_init(command_str, 0x0, process_num);

    int32_t counter;
    if (command[i] != '\n') { //parses out the argument for certain function calls such as cat
        for (counter = 0; counter < MAX_ARGS; counter++) {
            while (command[counter + i] == ' ') {i++;}
	        if (command[counter + i] == '\0' || command[counter + i] == '\n') {
	            new_pcb->args[counter] = '\0';
	            break;
	        } 
            new_pcb->args[counter] = command[counter + i];
	    }
    }

    if (read_dentry_by_name(command_str, &dentry_ref) == FAILURE) { //checks for failure from read_dentry_by_name
        new_pcb->parent_pcb->child_pcb = NULL;
        sti();
        return FAILURE;
    }
   
    // Acquire program data
    uint8_t data_buf[FILE_METADATA];
    if (read_data(dentry->inode_num, 0x0, data_buf, FILE_METADATA) == FAILURE) {
	    new_pcb->parent_pcb->child_pcb = NULL;
        sti();
        return FAILURE;
    }

    // Executable Check
    if (data_buf[0] != ELF_MAG0 || data_buf[1] != ELF_MAG1
        || data_buf[2] != ELF_MAG2 || data_buf[3] != ELF_MAG3) {
        new_pcb->parent_pcb->child_pcb = NULL;
        sti();
        return FAILURE;
    }
 
    // Set up Program paging
    uint32_t* program_page = get_page_directory(PROG_PAGE_IDX);
    uint32_t old_page = *program_page;
    uint32_t prog_mem = PHYS_PAGE_START + (PROGRAM_SIZE*process_num);
    *program_page = 0x00000000; // Clear the page just in case
    *program_page = prog_mem | FLAG_P | FLAG_RW | FLAG_US | FLAG_PS;
    
    // Flush the TLB
    asm volatile (
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        :
        :
        :"%eax"
    );

    // User-level program loader
    if (read_data(dentry->inode_num, 0x0, (uint8_t*)(PROG_CODE_START), FOUR_MB) == FAILURE) {
        *program_page = old_page;
        new_pcb->parent_pcb->child_pcb = NULL;
        sti();
        // CLEAR THE PCB STUFF AS WELL
        return FAILURE;
    }

    // Create the PCB
    set_pcb(new_pcb);
    avail_processes[process_num] = OCCUPIED;    // Claim the process  
    
    // Context Switch
    // ss0 will contain the location of the Kernel Data Segment
    tss.ss0 =  KERNEL_DS;
    // esp0 will point to the new stack pointer
    tss.esp0 = PHYS_PAGE_START - (process_num * PROG_STACK_SIZE) - LONG;
    
    sti();
    
    uint32_t eip = ((uint32_t*)(data_buf))[PROG_ENTRY_IDX];
    uint32_t result = execute_context_switch(eip); //calls context switch
    
    return result;
}


/* int32_t read(int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: This function takes a buffer and calls the function associated 
 *              with it based off the fd. It is the generic read function
 * INPUTS:  Fd = the file descriptor index into the file descriptor array
 *          Buf = the buffer to read from
 *          Nbytes = the number of bytes to read
 * OUTPUTS: Number of bytes read
 * SIDE EFFECTS: NONE
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    if (buf == NULL || fd < 0 || fd > FD_MAX) {return FAILURE;}
    pcb_t* pcb = find_pcb();
    if (pcb->file_array[fd].flags == 0) { 
        return FAILURE;
        } // Failure

    int32_t ret =  pcb->file_array[fd].operations_table[READ](fd,buf,nbytes);
    return ret;
}


/*Int 32_t write(int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: This function takes a buffer and calls the function associated 
 *              with it based off the fd. It is the generic write function
 * INPUTS:  Fd = the file descriptor index into the file descriptor array
 *          Buf = the buffer to write from
 *          Nbytes = the number of bytes to write
 * OUTPUTS: Number of bytes written
 * SIDE EFFECTS: NONE
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    if (buf == NULL || fd < 0 || fd > FD_MAX) {return FAILURE;}
    pcb_t* pcb = find_pcb();
    if (pcb->file_array[fd].flags == 0) { return FAILURE; } // Failure
    else { return pcb->file_array[fd].operations_table[WRITE](fd,buf,nbytes); }
}

/*
FUNCTION NAME: open
DESCRIPTION:   given filename, returns file descriptor in pcb if file exists
INPUTS:        filename - name of file to be opened
OUTPUTS:       returns file descriptor in pcb upon success or -1 upon failure
SIDE EFFECTS:  file is opened
*/
int32_t open(const uint8_t* filename) {
    // variable used for iteration
    int i;

    // directory entry
    dentry_t directory_entry;

    // file descriptor index for pcb 
    uint16_t fd;

    // get address of pcb
    pcb_t* pcb;
    pcb = find_pcb();

    // check if file exists using filename
    if(read_dentry_by_name(filename, &directory_entry) == FAILURE) {
        
        return FAILURE;

    }

    // iterate through pcb and store file in first avaiable slot
    for(fd = START; fd <= MAX_NUM_OF_FILES; fd++) {
        if(fd == MAX_NUM_OF_FILES) {
            return FULL;
        }
        else if(pcb->file_array[fd].flags == AVAILABLE) {
            pcb->file_array[fd].flags = OCCUPIED;
            pcb->file_array[fd].file_position = START;
            break;
        }
    }

    // fill in necessary information depending on file type
    for(i = START; i < NUM_OF_OPERATIONS; i++) {

        if(directory_entry.ftype == RTC_TYPE) {
            
            pcb->file_array[fd].operations_table[i] = rtc_operations_table[i];
            pcb->file_array[fd].inode = NULL; 

        }
        else if(directory_entry.ftype == DIRECTORY_TYPE) {
           
            pcb->file_array[fd].operations_table[i] = directory_operations_table[i];
            pcb->file_array[fd].inode = NULL; 

        }
        else if(directory_entry.ftype == FILE_TYPE) {
           
            pcb->file_array[fd].operations_table[i] = file_operations_table[i]; 
            pcb->file_array[fd].inode = directory_entry.inode_num;

        }

    }

    // open file using filename
    pcb->file_array[fd].operations_table[OPEN](filename);

    // return file descriptor
    return fd;
}

/*
FUNCTION NAME: close
DESCRIPTION:   given file descriptor, close file and update pcb appropriately
INPUTS:        fd - file descriptor of file to close
OUTPUTS:       returns 0 upon success or -1 upon failure
SIDE EFFECTS:  file is closed
*/
int32_t close(int32_t fd) {
    // variable used for iteration
    int i;

    // get address of pcb
    pcb_t* pcb;
    pcb = find_pcb();

     // if there is no file to close, return failure
    if(pcb->file_array[fd].flags == AVAILABLE) {
            
        return FAILURE;

    }

    // if file descriptor is not within range of valid file indices, return failure
    if((fd < MIN_NUM_OF_FILES) || (fd > MAX_NUM_OF_FILES)) {
        
        return FAILURE;

    }

    // update items in file entry
    for(i = START; i < NUM_OF_OPERATIONS; i++) {

        pcb->file_array[fd].operations_table[i] = NULL;

    }
    pcb->file_array[fd].inode = NONE;
    pcb->file_array[fd].file_position = NONE;
    pcb->file_array[fd].flags = AVAILABLE;
    
    // if closing the file properly is not successful, return failure
    /*if(pcb->file_array[fd].operations_table[CLOSE](fd) != SUCCESS) {
        
        return FAILURE;

    }*/

    // return success
    return SUCCESS;
}

/*
FUNCTION NAME: getargs
DESCRIPTION:   gets args for command
INPUTS:        buf - buffer to be written to
               nbytes - number of bytes to be written
OUTPUTS:       -1 for failure, 0 for success
SIDE EFFECTS:  moves arguement to buf
*/
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    if (buf == NULL || nbytes <= 0) {return FAILURE;}
    int32_t counter;
    pcb_t * pcb = find_pcb();
    if (pcb->args[0] == '\0') {return FAILURE;} //checks for no argument
    for (counter = 0; counter < nbytes; counter++) {
	    if (counter == MAX_ARGS - 1 && pcb->args[counter] != '\0') {return FAILURE;} //checks for valid arg
	    buf[counter] = pcb->args[counter];
	    if (pcb->args[counter] == '\0') {return SUCCESS;}
    }
    return FAILURE;
}

/*
FUNCTION NAME: vidmap
DESCRIPTION:   Maps a 4KB page to video memory so user-programs can use it
INPUTS:        screen_start - pointer to pointer which user wants to be set
                              to video memory
OUTPUTS:       screen_start - Will point to base addres of page which is set to
                              video memory
SIDE EFFECTS:  Creates a new 4KB page somewhere in the 132-136 MB virtual addresses
*/
int32_t vidmap(uint8_t** screen_start) {

    // only allow if the pointer is within user space
    if (screen_start == 0x00000000 || (uint32_t)screen_start <= EIGHT_MB) { return FAILURE; }
    
    // every process has its' own page for video memory
    uint32_t table_idx = find_pcb()->process_id;
    uint32_t* table = get_vidmem_entry(table_idx);
    
    // clear table entry, then set the appropriate flags and memory location
    *table = 0x00000000;
    *table = VIDEO_MEMORY | FLAG_P | FLAG_RW | FLAG_US;
    
    // flush the tlb
    asm volatile (
        "mov %%cr3, %%eax;"
        "mov %%eax, %%cr3;"
        :
        :
        :"%eax"
    );
    
    // set the screen_start pointer to point at the beginning of the page
    *screen_start = (uint8_t*)(VIDMEM_DIR_IDX * FOUR_MB + table_idx * FOUR_KB);
    
    return SUCCESS;
}

/*
FUNCTION NAME: set_handler
DESCRIPTION:   sets handler
INPUTS:        signum
			handler_address
OUTPUTS:       none
SIDE EFFECTS:  none
*/
int32_t set_handler(int32_t signum, void* handler_address) {
    return FAILURE;
}

/*
FUNCTION NAME: sigreturn
DESCRIPTION:   returns signal
INPUTS:        none
OUTPUTS:       none
SIDE EFFECTS:  none
*/
int32_t sigreturn(void) {
    return FAILURE;
}

// Returns the next available process, if any
int8_t next_available_process() {
    uint8_t process_num;
    for (process_num = 0; process_num < MAX_PROCESSES; process_num++) {
        if (avail_processes[process_num] == 0) { return process_num; } 
        // If we've found no available process, return failure
    }
    // If we don't find an available process, we've failed
    return FAILURE;

}
