#define ASM 1

.globl scheduler_switch_process

scheduler_switch_process:
    # Switch the kernel stack
    movl 4(%esp), %ebp
    # Return out of the previous scheduler
    leave
    ret


