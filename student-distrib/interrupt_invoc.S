# interrupt_invoc.S -   defines a few different functions to be used by
#                       interrupt descriptors

.globl common_interrupt
.globl exception_00_asm, exception_01_asm, exception_02_asm, exception_03_asm, exception_04_asm, exception_05_asm, exception_06_asm, exception_07_asm, exception_08_asm, exception_09_asm, exception_0A_asm, exception_0B_asm, exception_0C_asm, exception_0D_asm, exception_0E_asm, exception_0F_asm, exception_10_asm, exception_11_asm, exception_12_asm, exception_13_asm, exception_14_asm, exception_15_asm, exception_16_asm, exception_17_asm, exception_18_asm, exception_19_asm, exception_1A_asm, exception_1B_asm, exception_1C_asm, exception_1D_asm, exception_1E_asm, exception_1F_asm
.globl keyboard_handle, rtc_handle, syscall_handle, pit_handle

# common_interrupt
# DESCRIPTION:  Interrupt Invocation for all interrupts.
#               Will call the appropriate C function for
#               the interrupt
#common_interrupt:
#    pushal 
#    call *common_interrupt_jump_table(, %eax, 4)
#    popal
#    IRET

exception_00_asm:
    cli
    pushal
    call exception_00
    popal
    sti
    iret

exception_01_asm:
    cli
    pushal
    call exception_01
    popal
    sti
    iret

exception_02_asm:
    cli
    pushal
    call exception_02
    popal
    sti
    iret

exception_03_asm:
    cli
    pushal
    call exception_03
    popal
    sti
    iret

exception_04_asm:
    cli
    pushal
    call exception_04
    popal
    sti
    iret

exception_05_asm:
    cli
    pushal
    call exception_05
    popal
    sti
    iret

exception_06_asm:
    cli
    pushal
    call exception_06
    popal
    sti
    iret

exception_07_asm:
    cli
    pushal
    call exception_07
    popal
    sti
    iret

exception_08_asm:
    cli
    pushal
    call exception_08
    popal
    sti
    iret

exception_09_asm:
    cli
    pushal
    call exception_09
    popal
    sti
    iret

exception_0A_asm:
    cli
    pushal
    call exception_0A
    popal
    sti
    iret

exception_0B_asm:
    cli
    pushal
    call exception_0B
    popal
    sti
    iret

exception_0C_asm:
    cli
    pushal
    call exception_0C
    popal
    sti
    iret

exception_0D_asm:
    cli
    pushal
    call exception_0D
    popal
    sti
    iret

exception_0E_asm:
    cli
    pushal
    call exception_0E
    popal
    sti
    iret

exception_0F_asm:
    cli
    pushal
    call exception_0F
    popal
    sti
    iret

exception_10_asm:
    cli
    pushal
    call exception_10
    popal
    sti
    iret

exception_11_asm:
    cli
    pushal
    call exception_11
    popal
    sti
    iret

exception_12_asm:
    cli
    pushal
    call exception_12
    popal
    sti
    iret

exception_13_asm:
    cli
    pushal
    call exception_13
    popal
    sti
    iret

exception_14_asm:
    cli
    pushal
    call exception_14
    popal
    sti
    iret

exception_15_asm:
    cli
    pushal
    call exception_15
    popal
    sti
    iret

exception_16_asm:
    cli
    pushal
    call exception_16
    popal
    sti
    iret

exception_17_asm:
    cli
    pushal
    call exception_17
    popal
    sti
    iret

exception_18_asm:
    cli
    pushal
    call exception_18
    popal
    sti
    iret

exception_19_asm:
    cli
    pushal
    call exception_19
    popal
    sti
    iret

exception_1A_asm:
    cli
    pushal
    call exception_1A
    popal
    sti
    iret

exception_1B_asm:
    cli
    pushal
    call exception_1B
    popal
    sti
    iret

exception_1C_asm:
    cli
    pushal
    call exception_1C
    popal
    sti
    iret

exception_1D_asm:
    cli
    pushal
    call exception_1D
    popal
    sti
    iret

exception_1E_asm:
    cli
    pushal
    call exception_1E
    popal
    sti
    iret

exception_1F_asm:
    cli
    pushal
    call exception_1F
    popal
    sti
    iret

keyboard_handle:
    cli
    pushal
    call keyboard_interrupt_handle
    popal
    sti
    iret

rtc_handle:
    cli
    pushal
    call rtc_interrupt_handle
    popal
    sti
    iret

pit_handle:
    cli
    pushal
    call pit_interrupt_handle
    popal
    sti
    iret

syscall_handle:
    pushl %ebp
    movl %esp, %ebp
    cmpl $0x1, %eax # Make sure that the syscall number is valid
    jl syscall_fail 
    cmpl $0xA, %eax #checks if eax is within range
    ja syscall_fail
    decl %eax
    
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *syscall_jump_table(, %eax, 4)
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi

    popl %ebp
    iret

syscall_jump_table: .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

syscall_fail:
    movl $-1, %eax
    sti
    popl %ebp
    iret
