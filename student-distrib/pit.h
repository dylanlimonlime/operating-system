#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "i8259.h"
#include "syscall.h"
#include "x86_desc.h"
#include "idt.h"
#include "interrupt_invoc.h"
#include "scheduler.h"

// constants
#define PIT_INTERRUPT_LOCATION 0x20
#define IRQ_0                  0
#define CHANNEL_0              0x40
#define MODE_REGISTER          0x43
#define SQUARE_WAVE            0x36
#define INPUT_SIGNAL           1193182
#define RELOAD_VALUE           40
#define MASK                   0xFF
#define EIGHT                  8

// function to initialize pit
void pit_init();

// handler for pit interrupts
void pit_interrupt_handle();

#endif
