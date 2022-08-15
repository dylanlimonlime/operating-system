#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "lib.h"
#include "types.h"
#include "x86_desc.h"
#include "i8259.h"
#include "pcb.h"
#include "pcb_asm.h"
#include "paging.h"
#include "multi_term.h"
#include "scheduler_asm.h"
#include "syscall.h"
#include "keyboard.h"

#define PROG_PAGE_IDX   32
#define PHYS_PAGE_START (8*MEGABYTE)
#define PROGRAM_SIZE    (4*MEGABYTE)
#define PROG_STACK_SIZE (8*KILOBYTE)
#define NOT_ASSIGNED    -1
#define MAX_TERMINALS   3

// Stores the process numbers of the base shells of the terminals
int8_t terminal_process_nums[MAX_TERMINALS];

// Whichever terminal is currently running
uint8_t cur_scheduled_terminal;

void scheduler();

#endif
