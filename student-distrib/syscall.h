#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "filesys.h"
#include "paging.h"
#include "x86_desc.h"
#include "lib.h"
#include "syscall_asm.h"
#include "filesys.h"
#include "types.h"
#include "rtc.h"
#include "terminal.h"
#include "pcb.h"

// File signatures for ELF
#define ELF_MAG0            0x7F
#define ELF_MAG1            0x45
#define ELF_MAG2            0x4C
#define ELF_MAG3            0x46

// constants used for addresses
#define VIRT_PAGE_START    (128*MEGABYTE)
#define PROG_PAGE_IDX       32
#define PHYS_PAGE_START    (8*MEGABYTE)
#define PROG_CODE_START     0x08048000
#define PROG_STACK_SIZE    (8*KILOBYTE)
#define PROGRAM_SIZE       (4*MEGABYTE)
#define PCB_MEM_SIZE       (8*KILOBYTE)

// mask used for pcb
#define PCB_MASK            0xFFFFE000

// constants for function outputs
#define SUCCESS             0
#define FAILURE            -1

// general constants for use in functions
#define NONE               -1
#define FULL               -1
#define AVAILABLE           0
#define OCCUPIED            1
#define START               0
#define PROG_ENTRY_IDX      6
#define FILE_METADATA       30
#define MAX_FILES           8
#define VIDMEM_PG_IDX_OFF   8

// constants for file types
#define RTC_TYPE            0
#define DIRECTORY_TYPE      1
#define FILE_TYPE           2

// constants for operations
#define NUM_OF_OPERATIONS   4
#define READ                0
#define WRITE               1
#define OPEN                2
#define CLOSE               3

// Maximum number of processes in a single terminal
#define PROCESS_CHAIN_MAX   4


// system calls
extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

// Helpers
extern int8_t next_available_process();

#endif
