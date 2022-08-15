
#define ASM 1

.globl scheduler_save_reg

scheduler_save_reg:
    pushl %ebp
    call save_scheduler_reg
    addl $4, %esp
    ret
