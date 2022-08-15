/* exception.c - Contains functions to address all 32 
 * exceptions defined by Intel for use with the IDT. 
 * Exceptions are sourced from Table 5-1 in the Intel
 * Architecture Software Developer's Manual Volume 3: 
 * System Programming Guide
 */

#ifndef _EXCEPTION_C
#define _EXCEPTION_C
#include "exception.h"
#include "lib.h"

void exception_00() {
    printf("EXCEPTION 00: DIVIDE BY ZERO\n");
    halt(EXCEPTION);
}

void exception_01() {
    printf("EXCEPTION 01: RESERVED FOR INTEL USE\n");
    halt(EXCEPTION);
}

void exception_02() {
    printf("EXCEPTION 02: NON-MASKABLE INTERRUPT (NMI)\n");
    halt(EXCEPTION);
}

void exception_03() {
    printf("EXCEPTION 03: KGDB BREAKPOINT\n");
    halt(EXCEPTION);
}

void exception_04() {
    printf("EXCEPTION 04: OVERFLOW\n");
    halt(EXCEPTION);
}

void exception_05() {
    printf("EXCEPTION 05: BOUND RANGE EXCEEDED\n");
    halt(EXCEPTION);
}

void exception_06() {
    printf("EXCEPTION 06: INVALID OPCODE\n");
    halt(EXCEPTION);
}

void exception_07() {
    printf("EXCEPTION 07: DEVICE NOT AVAILABLE (NO MATH COPROCESSOR)\n");
    halt(EXCEPTION);
}

void exception_08() {
    printf("EXCEPTION 08: DOUBLE FAULT\n");
    halt(EXCEPTION);
}

void exception_09() {
    printf("EXCEPTION 09: COPROCESSOR SEGMENT OVERRUN\n");
    halt(EXCEPTION);
}

void exception_0A() {
    printf("EXCEPTION 0A: INVALID TSS\n");
    halt(EXCEPTION);
}

void exception_0B() {
    printf("EXCEPTION 0B: SEGMENT NOT PRESENT\n");
    halt(EXCEPTION);
}

void exception_0C() {
    printf("EXCEPTION 0C: STACK-SEGMENT FAULT\n");
    halt(EXCEPTION);
}

void exception_0D() {
    printf("EXCEPTION 0D: GENERAL PROTECTION FAULT\n");
    halt(EXCEPTION);
}

void exception_0E() {
    printf("EXCEPTION 0E: PAGE FAULT\n");
    halt(EXCEPTION);
}

void exception_0F() {
    printf("EXCEPTION 0F: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_10() {
    printf("EXCEPTION 10: x87 FPU FLOATING-POINT ERROR (MATH FAULT)\n");
    halt(EXCEPTION);
}

void exception_11() {
    printf("EXCEPTION 11: ALIGNMENT CHECK\n");
    halt(EXCEPTION);
}

void exception_12() {
    printf("EXCEPTION 12: MACHINE CHECK\n");
    halt(EXCEPTION);
}

void exception_13() {
    printf("EXCEPTION 13: SIMD FLOATING-POINT EXCEPTION\n");
    halt(EXCEPTION);
}

void exception_14() {
    printf("EXCEPTION 14: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_15() {
    printf("EXCEPTION 15: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_16() {
    printf("EXCEPTION 16: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_17() {
    printf("EXCEPTION 17: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_18() {
    printf("EXCEPTION 18: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_19() {
    printf("EXCEPTION 19: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1A() {
    printf("EXCEPTION 1A: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1B() {
    printf("EXCEPTION 1B: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1C() {
    printf("EXCEPTION 1C: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1D() {
    printf("EXCEPTION 1D: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1E() {
    printf("EXCEPTION 1E: INTEL RESERVED\n");
    halt(EXCEPTION);
}

void exception_1F() {
    printf("EXCEPTION 1F: INTEL RESERVED\n");
    halt(EXCEPTION);
}


#endif
