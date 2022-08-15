#define ASM 1

#include "x86_desc.h"


#define PROGRAM_START   0x083FFFFC
#define LONG_SIZE       4
#define INT_ENABLE      0x200
#define ZERO            0x00000000

.globl execute_context_switch
.globl halt_return

/* halt_return
 * DESCRIPTION: Assembly-level return to execute for halt
 * INPUTS: status - status passed in by function calling halt 
 * OUTPUTS: None
 * RETURNS: status
 * SIDE EFFECTS: Switches to parent program stack and instructions
 */
halt_return:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax #8 is used to obtain the first argument. status
    movl 12(%ebp), %ebx #12 obtains the ebp from the argument of the function call
    movl 16(%ebp), %ecx #16 obtains the esp form the argument of the function call
    movl %ebx, %ebp
    movl %ecx, %esp
   
    leave
    ret
 

/* execute_context_switch
 * DESCRIPTION: Runs context switch for execute, switching to new program's
 *              stack and instructions
 * INPUTS: NONE
 * OUTPUTS: NONE
 * RETURNS: NONE
 * SIDE EFFECTS: Switches to new program stack and instructions, and user-space
 *               if this is the initial execute of shell
 */
execute_context_switch:
    movl 4(%esp), %ebx  # Retrieve the eip since 4 obtains the first argument of the function call
    xorl %eax, %eax

    # User DS
    movl $USER_DS, %eax
    pushl %eax
    movw %ax, %ds
    
    # ESP - Has to point to bottom of new program stack
    movl $PROGRAM_START, %eax
    pushl %eax

    # EFLAG
    pushfl
    popl %eax
    orl $INT_ENABLE, %eax   # Ensure interrupts are enabled
    pushl %eax

    # CS
    movl $USER_CS, %eax
    pushl %eax

    # EIP
    pushl %ebx

    pushl %esp
    pushl %ebp
    call add_reg
    addl $8, %esp #adds 8 to move the stack pointer to counteract the two pushed for the call

    iret

