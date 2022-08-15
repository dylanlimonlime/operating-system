#include "terminal.h"

/* int32_t terminal_open(const uint8_t* filename)
 * DESCRIPTION: opens up terminal driver for use by other functions
 * INPUT: 
 * filename: the filename of which the terminal is opened to
 * OUTPUT: returns -1 for failure
 * SIDE EFFECTS: opens up terminal
 */

int32_t terminal_open(const uint8_t* filename) {
    return 1; //returns -1 for failure
}

/* int32_t terminal_close(int32_t fd)
 * DESCRIPTION: closes up terminal for other functions
 * INPUT: 
 * fd: file directory to close
 * OUTPUT: returns -1 for failure
 * SIDE EFFECTS: Closes file directory
 */

int32_t terminal_close(int32_t fd) {
    return -1; //returns -1 for failure
}

/* int32_t terminal_read(int32_t fd,void* buf, int32_t nbytes))
 * DESCRIPTION: reads the input on the keyboard, only stops when enter is pressed
 * INPUT: 
 * fd - file directory for the terminal
 * buf- buffer to read from
 * nbytes- number of bytes to copy over to a copy buffer
 * OUTPUT: returns the number of characters typed
 * SIDE EFFECTS: Reads the terminal and copies it to a buffer
 */

int32_t terminal_read(int32_t fd,void* buf, int32_t nbytes) {
    while (!enter[cur_scheduled_terminal]) {} //continues reading until ENTER is pressed

    char* char_buf = (char*)buf;
    int size, i; //intializes the counter for reading
    for (size = 0; size < nbytes; size++) { //runs through nbytes to read
        char_buf[size] = keyboard_buf[size]; //adds it to a copy buffer since keyboard_buf will be cleared
        if (keyboard_buf[size] == '\0' || keyboard_buf[size] == '\n') { break; }
    }
    for (i = 0; i < BUFFER_LIMIT; i++) {keyboard_buf[i] = '\0';} //clears buffer with \0
    enter[cur_scheduled_terminal] = 0;
    return size + 1;
}

/* int32_t terminal_write(int32_t fd,void* buf, int32_t nbytes))
 * DESCRIPTION: writes the indicated buf to keyboard
 * INPUT: 
 * fd - file directory for the terminal
 * buf- buffer to write from
 * nbytes- number of bytes to write
 * OUTPUT: returns the number of characters written
 * SIDE EFFECTS: Reads the terminal and writes it to screen
 */

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    int counter;
    char* char_buf = (char*) buf;
    for (counter = 0; counter < nbytes; counter++) {
	    putc(char_buf[counter]);
    }
    return counter;
}
/* int32_t terminal_fail(fd, buf, nbytes)
 * DESCRIPTION: Returns failure if incorrect terminal is called
 * INPUT: 
 * FD: file descriptor
 * buf: the buffer
 * nbytes: the number of bytes
 * OUTPUT: -1 to indicate failure
 * SIDE EFFECTS: NONE
 */
int32_t terminal_fail(int32_t fd, const void* buf, int32_t nbytes) {return -1;}
