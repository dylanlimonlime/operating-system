/* lib.c - Some basic library functions (printf, strlen, etc.)
 * 
 */

#include "lib.h"

#define CURSOR_CONTROL 0x3D4
#define CURSOR_DATA 0x3D5
#define MASK 0xFF
#define CURSOR_TYPE 0x0F

//int screen_x;
//int screen_y;
//char* video_mem = (char *)VIDEO;

/* uint8_t is_power_2()
 * DESCRIPTION: Checks if number is power of 2
 * INPUTS: x - Unsigned number to be checked
 * OUTPUTS: NONE
 * RETURNS: Index of active bit (i.e. what power of 2 it is), -1 if not power of 2
 * SIDE EFFECTS: NONE
 */
int8_t is_power_2(uint16_t x) {
	int8_t result = -1;
	uint8_t i, temp;
	for (i = 0; i < 16; i++) {  // Go through all 16 bits
		temp = x >> i;          
		if (temp & 0x1) {       // Check if bit is 1 or 0
            // If no other bit so far has been 1, save that index in result
            if (result == -1) { result = i; }
            // If another bit has been found to be 1, it's not a power of 2
			else { return -1; } // Break early because efficiency
		}
	}
	return result;
}

/* void set_cursor()
 * Inputs:
 * x = new x position increment of cursor
 * y = new y position increment of cursor
 * Outputs: NONE
 * Side Effect: changes cursor on screen
 */
void set_cursor(int8_t x, int8_t y) {
    sscreenx[cur_terminal] += x;
    sscreeny[cur_terminal] += y;
    if (x == 0 && y == 0) {sscreenx[cur_terminal] = 0; sscreeny[cur_terminal] = 0;}
}

/* void text_colour(int8_t text, int8_t background)
 * DESCRIPTION: sets text colour to text and background color to background
 * INPUTS:
 * text = text colour
 * background: background colour
 * OUTPUTS: NONE
 * SIDE EFFECTS: changes text colour
 */
void text_colour(int8_t text, int8_t background) {
    ATTRIB = text + (background << BACKGROUND);  //shifts by 4 to place background colour into upper 4 bits on ATTRIB
}

/* void clear_all() 
* INPUTS : NONE
* OUTPUTS : NONE
* SIDE EFFECT : clears screen and sets cursor to 0,0
*/
void clear_all() {
    clear();
    sscreenx[cur_terminal] = 0;
    sscreeny[cur_terminal] = 0;
    flashy_set(0);
}

/* void flashy_set(unint16_t position)i
 * DESCRIPTION: sets the flashing cursor to the position indicated by the function parameter
 * INPUTS: position of the cursor in row major order
 * OUTPUTS: NONE
 * SIDE EFFECT: Sets the flashing cursor to the postion
 */
void flashy_set(uint16_t position) {
	outb(CURSOR_TYPE, CURSOR_CONTROL);
	outb((uint8_t) (position & MASK), CURSOR_DATA);
	outb(0x0E, CURSOR_CONTROL); //0x0E tells CURSOR_CONTROL to be on
	outb((uint8_t) ((position >> 8) & MASK), CURSOR_DATA); //8 bitshifts position to correct location
 	
}

/* uint16_t get_flashy()
 * DESCRIPTION: returns the row major coordinates of the flashing cursor
 * INPUTS : NONE
 * OUTPUTS : position of flashing cursor
 * SIDE EFECTS: NONE
 */
uint16_t get_flashy() {
    uint16_t pos = 0;
	outb(CURSOR_TYPE, CURSOR_CONTROL);
	pos |= inb(CURSOR_DATA);
	outb(0x0E, CURSOR_CONTROL); //0x0E tells CURSOR_CONTROL to be on
	pos |= ((uint16_t)inb(CURSOR_DATA)) << 8; //8 bitshifts position to correct location
	return pos;
}

/* void vertical_scroll()
 * DESCRIPTION: scrolls the screen when the final position is reached on screen
 * or an enter is placed on final row
 * INPUTS: NONE
 * OUTPUTS: NONE
 * SIDE EFFECTS: scrolls screen
 */
void vertical_scroll() {
    memmove(video_mem + VIDEO_MEM_OFFSET, video_mem + VIDEO_MEM_OFFSET + VIDEO_SPACE_PER_ROW, TOTAL_VIDEO_SPACE);
    int32_t j;
	for(j = 0; j < NUM_COLS; j++){
		*(uint8_t *)(video_mem + VIDEO_MEM_OFFSET + ((NUM_COLS*(NUM_ROWS - 1) + j) << 1)) = ' ';
		*(uint8_t *)(video_mem + VIDEO_MEM_OFFSET + ((NUM_COLS*(NUM_ROWS - 1) + j) << 1) + 1) = ARRTIB;;
	}

}

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
    
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}



/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}
/* void putc_shell(uint8_t c);
 * DESCRIPTION: The same as putc, but it only works in the keyboard handler. It will only print 
 * to the current screen
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the current terminal only and calls vertical scroll at 
 *  end of screen as well as new line at end of row
 */

void putc_shell(uint8_t c) {
    if(c == '\n' || c == '\r') {
	if (sscreeny[cur_terminal] == NUM_ROWS - 1) {vertical_scroll_shell();}
	//scrolls vertically if enter is pressed while at final row, which is NUM_ROWS - 1 
	else {sscreeny[cur_terminal]++;}
        sscreenx[cur_terminal] = 0;
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS *sscreeny[cur_terminal] + sscreenx[cur_terminal]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS *sscreeny[cur_terminal] + sscreenx[cur_terminal]) << 1) + 1) = ATTRIB;
        sscreenx[cur_terminal]++;
	if (sscreenx[cur_terminal] == NUM_COLS) { //if screen_x hits end of row, place newline 
	if (sscreeny[cur_terminal] == NUM_ROWS - 1) {vertical_scroll_shell();}
	//if at final row, which is at NUM_ROWS - 1, vertical scroll as well
	else {sscreeny[cur_terminal]++;}
	sscreenx[cur_terminal] = 0;
	}
	sscreenx[cur_terminal] %= NUM_COLS;
        sscreeny[cur_terminal] %= NUM_ROWS;	
    }
    flashy_set(sscreenx[cur_terminal] + sscreeny[cur_terminal]*NUM_COLS); //sets cursor to next screen positio
}

/* void vertical_scroll_shell()
 * DESCRIPTION: scrolls the screen when the final position is reached on screen
 * or an enter is placed on final row. The same as the regular vertical scroll, but only works for the current screen
 * INPUTS: NONE
 * OUTPUTS: NONE
 * SIDE EFFECTS: scrolls screen
 */
void vertical_scroll_shell() {
    memmove(video_mem, video_mem + VIDEO_SPACE_PER_ROW, TOTAL_VIDEO_SPACE);
    int32_t j;
	for(j = 0; j < NUM_COLS; j++){
		*(uint8_t *)(video_mem + ((NUM_COLS*(NUM_ROWS - 1) + j) << 1)) = ' ';
		*(uint8_t *)(video_mem + ((NUM_COLS*(NUM_ROWS - 1) + j) << 1) + 1) = ATTRIB;;
        }


}
/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console and calls vertical scroll at 
 *  end of screen as well as new line at end of row
 */
void putc(uint8_t c) {
    if(c == '\n' || c == '\r') {
	if (sscreeny[cur_scheduled_terminal] == NUM_ROWS - 1) {vertical_scroll();}
	//scrolls vertically if enter is pressed while at final row, which is NUM_ROWS - 1 
        else {sscreeny[cur_scheduled_terminal]++;}
        sscreenx[cur_scheduled_terminal] = 0;
    } else {
        *(uint8_t *)(video_mem + VIDEO_MEM_OFFSET + ((NUM_COLS *sscreeny[cur_scheduled_terminal] + sscreenx[cur_scheduled_terminal]) << 1)) = c;
	if (TERMINAL_FLAG) {*(uint8_t *)(video_mem + VIDEO_MEM_OFFSET + ((NUM_COLS *sscreeny[cur_scheduled_terminal] + sscreenx[cur_scheduled_terminal]) << 1) + 1) = ARRTIB;}
	else {*(uint8_t *)(video_mem + VIDEO_MEM_OFFSET + ((NUM_COLS *sscreeny[cur_scheduled_terminal] + sscreenx[cur_scheduled_terminal]) << 1) + 1) = ATTRIB;}

	sscreenx[cur_scheduled_terminal]++;
	if (sscreenx[cur_scheduled_terminal] == NUM_COLS) { //if screen_x hits end of row, emplace newline 
	if (sscreeny[cur_scheduled_terminal] == NUM_ROWS - 1) {vertical_scroll();}
	else {sscreeny[cur_scheduled_terminal]++;}
        sscreenx[cur_scheduled_terminal] = 0;
	    }
      }
    sscreenx[cur_scheduled_terminal] %= NUM_COLS;
    sscreeny[cur_scheduled_terminal] %= NUM_ROWS;
 
    if (CURSOR)flashy_set(sscreenx[cur_scheduled_terminal] + sscreeny[cur_scheduled_terminal]*NUM_COLS); //sets cursor to next screen position
    
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}
