#include "keyboard.h"
#include "types.h"
#include "terminal.h"
#include "multi_term.h"

//a global array of chars that convert the scanline into a printable char. A capital X implies
//a keypress that can't be represented easily on screen ie. backspace. 64 is the number of keys
char scan_to_char[64] = {
'\0','1','2','3','4','5','6','7','8','9','0','-','=','\0',' ','q','w','e','r','t','y','u','i','o',
'p','[',']','\n','\0','a','s','d','f','g','h','j','k','l',';','\'','`','\0','\\','z','x','c','v','b',
'n','m',',','.','/','\0','\0','\0',' ','\0','\0','\0','\0','\0','\0','\0'
};
//a global array of chars that convert the scanline into a printable char. A capital X implies
//a keypress that can't be represented easily on screen ie. backspace. 64 is the number of keys
char CAPS_scan_to_char[64] = {
'\0','1','2','3','4','5','6','7','8','9','0','-','=','\0',' ','Q','W','E','R','T','Y','U','I','O',
'P','[',']','\n','\0','A','S','D','F','G','H','J','K','L',';','\'','`','\0','\\','Z','X','C','V','B',
'N','M',',','.','/','\0','\0','\0',' ','\0','\0','\0','\0','\0','\0','\0'
};
//a global array of chars that convert the scanline into a printable char. A capital X implies
//a keypress that can't be represented easily on screen ie. backspace. 64 is the number of keys
char SHIFT_scan_to_char[64] = {
'\0','!','@','#','$','%','^','&','*','(',')','_','+','\0',' ','Q','W','E','R','T','Y','U','I','O',
'P','{','}','\n','\0','A','S','D','F','G','H','J','K','L',':','\"','~','\0','|','Z','X','C','V','B',
'N','M','<','>','?','\0','\0','\0',' ','\0','\0','\0','\0','\0','\0','\0'
};
//a global array of chars that convert the scanline into a printable char. A capital X implies
//a keypress that can't be represented easily on screen ie. backspace. 64 is the number of keys
char SHIFT_CAPS_scan_to_char[64] = {
'\0','!','@','#','$','%','^','&','*','(',')','_','+','\0',' ','q','w','e','r','t','y','u','i','o',
'p','{','}','\n','\0','a','s','d','f','g','h','j','k','l',':','\"','~','\0','|','z','x','c','v','b',
'n','m','<','>','?','\0','\0','\0',' ','\0','\0','\0','\0','\0','\0','\0'
};
/* Keyboard_init()
*  Description: This functions initliazes the port inn the IDT for the keyboard
*  as well as IRQ1 for the PIC
*  Inputs: NONE
*  Outputs: NONE
*  Side Effects: Initializes keyboard and allows keys to be pressed and acknowledged
*/
void keyboard_init() {
    // Set the port appropriately
    set_idt_gate(KEYBOARD_PIC_PORT, (uint32_t)keyboard_handle, KERNEL_CS, GATE_SIZE_32, KERNEL_PRIV, GATE_PRESENT, INTERRUPT_GATE);
    enable_irq(KEYBOARD_PORT); //sets the keyboard to be at irq1
    CAPS = 0; //initializes caps boolean to zero
    SHIFT = 0; //initializes caps boolean to zero
    CTRL = 0; //intializes the CTRL boolean to zero
    TERMINATE = 0; //initialzes the TERMINATE boolean to zero
    ALT = 0;
    cur_keyboard = keyboard_buf_0;
    return;
}

/* keyboard_get_scanline()
*  Description: This functions takes the keyboard input and obtains the scanline of it
*  Inputs: NONE
*  Outputs: returns the scanline of the key pressed/depressed
*  Side effects: flushes the keyboard and allows another key to be pressed
*/
uint8_t keyboard_get_scanline() {
    uint8_t result = inb(DATA_PORT); //obtains the scanline from the Keyboard portion
    if (result == CONTROL) {CTRL = 1; leave();}
    //implement tab and alt
    else if (CTRL && result == l_scanline) {clear_all();leave();} //clears screen and resets cursor if left control pressed
    else if (CTRL && result == c_scanline) {leave(); halt(0);} //restarts shell is control c is pressed
    else if (result == CONTROL_DEPRESS) {CTRL = 0; leave();} 
    else if (result == CAPS_LOCK) {CAPS ^= 1; leave();} //reversed the caps boolean based of how many times caps is pressed
    else if (result == LEFT_SHIFT_PRESS || result == RIGHT_SHIFT_PRESS) {SHIFT = 1; leave();} //sets shift on is pressed
    else if (result == LEFT_SHIFT_DEPRESS || result == RIGHT_SHIFT_DEPRESS) {SHIFT = 0; leave();} //sets shift off when depressed
    else if (ALT && (result == F1 || result == F2 || result == F3)) {leave(); swap_terminal(result - F1);}
    else if (result == F5) { leave(); scheduler(); }
    else if (result == LEFT_ALT_PRESS) { ALT = 1; leave(); }
    else if (result == LEFT_ALT_DEPRESS) { ALT = 0; leave(); }
    else if (result == BACKSPACE) {
        if(buf_index[cur_terminal] != 0) { //wont work if the index is at 0
	    set_cursor(-1,0); //moves cursor back by 1, or -1
	    putc_shell(' '); //replces it with an empty space
            set_cursor(-1,0); //moves cursor back by 1, or -1
	    (buf_index[cur_terminal])--; //decrements buf_index by 1
	    flashy_set(get_flashy() - 1); //decrements flashing cursor by 1
	    leave(); //indicated end of signal function
	    }
    } // moves cursor back by one, puts a space, and moves curosr back by one to simulate a backspace. -1 is the offset to move x position back by one
    //else if (result == TAB && buf_index[cur_terminal] > TAB_LIMIT) {leave();}
    else if (result != BACKSPACE && (buf_index[cur_terminal] == KEYBOARD_LIMIT && (result != ENTER_PRESS))) {leave();}
    else if (result >= KEY_PASS) {leave();}
    return result;
}

/* keyboard_get_char(uint8_t scanline)
 *  Description: This functions takes the key scanline and converts it into a char
 *  Inputs: the scanline of the key pressed
 *  Outputs: returns the char representation of the key pressed
 *  Side Effects: NONE
 */
char keyboard_get_char(uint8_t scanline) {
    char result;
    if (CAPS && !SHIFT) {result = CAPS_scan_to_char[(int)(scanline - 1)];} //uses array if caps on but shift isnt
    else if (CAPS && SHIFT) {result = SHIFT_CAPS_scan_to_char[(int)(scanline - 1)];} //uses array if caps and shift pressed
    else if (!CAPS && SHIFT) {result = SHIFT_scan_to_char[(int)(scanline - 1)];} //uses array is shift pressed but caps isnt
    else {result = scan_to_char[(int)(scanline - 1)];} //takes the scanline and converts it to char representation
    // a one is subtracted here to base the index
    if (result == '\0') {leave();} //doesn't print out scanline if result is null character
    return result;
}

/* keyboard_interrupt_handle()
*  Description: This functions obtains the scanline char,
*  converts it into a visual char, and displays it
*  Inputs: NONE
*  Outputs: NONE
*  Side Effects: Displays char into kernel screen
*/
void keyboard_interrupt_handle() {	
    uint8_t scanline = keyboard_get_scanline(); //gets the scanline of the keypress
    char key = keyboard_get_char(scanline); //finds the char represenation of the scanline
 
    if (!TERMINATE) {

	//int8_t tab_loop = 1;    //initializes the tab loop to 1 because only 1 char is written
	//if (key == '\t') {  //if tab is 4
	//    tab_loop = 4; //when tab is pressed, 4 spaces have to be written, so tab_loop is set to 4
	//    key = ' '; //sets key to space to simulate a tab
	//} 
	
	//while (tab_loop != 0) { //runs until tab_loop is 0 to simulate a counter
        line_buffered_input(key); //places key onto screen and places it onto buffer
	            putc_shell(key);
                //putc(key);
	//    tab_loop--;
	//}

        if (key == '\n') {
	    //ENTER = 1; //sets ENTER flag to 1 to indicate that ENTER is pressed
	    enter[cur_terminal] = 1;
	//    buf_index_copy[cur_terminal] = buf_index[cur_terminal]; //copies buf_index over to copy
	    buf_index[cur_terminal] = 0; //sets the buf_index to zero to restart buffer
	}

        leave();
    }
    TERMINATE = 0; //sets terminate to 0 to indicate that next char can be written
}

/* void leave()
 * DESCRIPTION: sends EOI to the keyboard port, and tells the computer not to print or add to buffer
 * INPUT: NONE
 * OUTPUT: NONE
 * SIDE EFFECTS: prevents the key from printing or being added to buffer
 */
void leave() {
    send_eoi(KEYBOARD_PORT);
    TERMINATE = 1; //sets TERMINATE to 1 to indicate no screen output
}

/* void line_buffered_input(char c)
 * DESCRIPTION: adds the key to the buffer and increments the buffer index
 * INPUTS: 
 * char c- the char to add to the buffer
 * OUTPUTS: NONE
 * SIDE EFFECTS: adds the key to the buffer and increments the index
 */
void line_buffered_input(char c) {	
    cur_keyboard[(buf_index[cur_terminal])++] = c; //adds key to buffer and increments the index
}

