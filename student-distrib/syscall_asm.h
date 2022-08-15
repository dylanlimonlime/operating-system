#ifndef _SYSCALL_ASM_H
#define _SYSCALL_ASM_H

extern uint32_t execute_context_switch(); 

extern uint32_t halt_return(uint8_t status, uint32_t ebp, uint32_t esp);

#endif
