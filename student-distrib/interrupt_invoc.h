#ifndef _INTERRUPT_INVOC_H
#define _INTERRUPT_INVOC_H

//extern void common_interrupt(); 
extern void exception_00_asm();
extern void exception_01_asm();
extern void exception_02_asm();
extern void exception_03_asm();
extern void exception_04_asm();
extern void exception_05_asm();
extern void exception_06_asm();
extern void exception_07_asm();
extern void exception_08_asm();
extern void exception_09_asm();
extern void exception_0A_asm();
extern void exception_0B_asm();
extern void exception_0C_asm();
extern void exception_0D_asm();
extern void exception_0E_asm();
extern void exception_0F_asm();
extern void exception_10_asm();
extern void exception_11_asm();
extern void exception_12_asm();
extern void exception_13_asm();
extern void exception_14_asm();
extern void exception_15_asm();
extern void exception_16_asm();
extern void exception_17_asm();
extern void exception_18_asm();
extern void exception_19_asm();
extern void exception_1A_asm();
extern void exception_1B_asm();
extern void exception_1C_asm();
extern void exception_1D_asm();
extern void exception_1E_asm();
extern void exception_1F_asm();

extern void keyboard_handle();

extern void rtc_handle();

extern void syscall_handle();

extern void pit_handle();

#endif
