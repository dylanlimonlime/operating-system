#ifndef _MULTI_TERM_H
#define _MULTI_TERM_H

#include "types.h"
#include "lib.h"

#include "paging.h"
#include "syscall.h"
#include "pcb.h"

#define MAX_TERMINALS 3

#define VMEM_IDX        0xB8
#define VMEM            0xB8000
#define VMEM_BAK_BASE_IDX  0xB9
#define VMEM_BAK_BASE_ADDR  0xB9000

uint8_t cur_terminal;
//uint8_t cur_scheduled_terminal;

int8_t swap_terminal(uint8_t new_terminal);

//uint16_t sscreen[3];
int32_t buffer_holder[3];
uint8_t enter[3];
int32_t sscreenx[3];
int32_t sscreeny[3];



#endif
