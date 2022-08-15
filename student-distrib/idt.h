/* idt.h - Header file for Interrupt Descriptor Table
 */

#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "interrupt_invoc.h"

#define KERNEL_PRIV 0x0
#define USER_PRIV 0x3

#define GATE_SIZE_32 0x1
#define GATE_SIZE_16 0x0

#define GATE_PRESENT 0x1
#define GATE_NOT_PRESENT 0x0 

#define TASK_GATE 0x0
#define INTERRUPT_GATE 0x1
#define TRAP_GATE 0x2

#define SYSCALL_VECTOR 0x80

/* Populates the IDT with empty descriptors */
void setup_idt();

/* Fully initializes the IDT, initializing all values and making the exceptions */ 
extern void idt_init();

/* Creates all of the exception descriptors */
void idt_exception_setup();

/* Will set up an idt descriptor gate to the specified parameters */
extern void set_idt_gate(int idt_idx, uint32_t offset, uint16_t segment, uint32_t gate_size, uint32_t gate_dpl, uint32_t gate_presence, uint32_t gate_type);

/* Returns a function pointer to the interrupt invocation in question */
extern uint32_t get_interrupt_invoc(uint32_t invoc);

#endif
