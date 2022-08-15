
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "idt.h"
#include "i8259.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "scheduler.h"

#define KEYBOARD_PIC_PORT 0x21

// Define the various ports
#define DATA_PORT 0x60
#define STATUS_REGISTER 0x64
#define COMMAND_REGISTER 0x64

// Define commands to be used 
#define WRITE_CONTROLLER_CONFIGURATION 0x60
#define DISABLE_PORT 0xAD
#define ENABLE_PORT 0xAE

// Controller configuration to enab
#define CONTROLLER_CONFIG 0x45

#define KEYBOARD_PORT 0x01

//keypress definitions
#define ENTER_PRESS 0x1C
#define KEY_PASS 0x3B
#define CAPS_LOCK 0x3A
#define LEFT_SHIFT_PRESS 0x2A
#define RIGHT_SHIFT_PRESS 0x36
#define LEFT_SHIFT_DEPRESS 0xAA
#define RIGHT_SHIFT_DEPRESS 0xB6
#define CONTROL 0x1D
#define CONTROL_DEPRESS 0x9D
#define BACKSPACE 0x0E
#define TAB 0x0F
#define LEFT_ALT_PRESS      0x38
#define LEFT_ALT_DEPRESS    0xB8
#define F_ONE       0x3B
#define F_TWO       0x3C
#define F_THREE     0x3D
#define l_scanline 0x26
#define c_scanline 0x2E
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define F5 0x3F

#define TAB_LIMIT 123
#define KEYBOARD_LIMIT 127
#define BUFFER_LIMIT 128

char prev_echo_char; //a global variable holding the previous char
char echo_char; //a global variable holding the current char

char scan_to_char[64]; //a global array that holds all the chars when neither shift nor caps are pressed
char CAPS_scan_to_char[64]; //a global array that holds all the chars when CAPS is pressed
char SHIFT_scan_to_char[64]; //a global array that holds all the chars when SHIFT is pressed
char SHIFT_CAPS_scan_to_char[64]; //a global array that holds all the chars when SHIFT and CAPS is pressed
char* keyboard_buf; //buffer of the keyboard. 128 bytes max
char* cur_keyboard;
char keyboard_buf_0[BUFFER_LIMIT];
char keyboard_buf_1[BUFFER_LIMIT];
char keyboard_buf_2[BUFFER_LIMIT];
uint8_t buf_index[3];
uint8_t buf_index_copy[3];
uint8_t CAPS; //a boolean for if caps is pressed
uint8_t SHIFT; //a boolean for if shift is pressed
uint8_t CTRL; //a boolean for if control is pressed
uint8_t ENTER;
uint8_t TERMINATE;
uint8_t ALT;
extern void keyboard_init(); //initializes the keyboard

uint8_t keyboard_get_scanline(); //gets the scanline of the keyboard

char keyboard_get_char(uint8_t scanline); //gets the char of the keyboard

extern void keyboard_interrupt_handle(); //handles the interrupt of the keyboard

extern void leave();

extern void line_buffered_input(char c);

#endif



