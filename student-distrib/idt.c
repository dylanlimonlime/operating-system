
#ifndef _IDT_C
#define _IDT_C
#include "x86_desc.h"
#include "idt.h"
#include "exception.h"
#include "interrupt_invoc.h"

/* Holds the function pointers of all the exception interrupt invocations */
static uint32_t interrupt_invocation[NUM_VEC] = {
    (uint32_t)exception_00_asm, (uint32_t)exception_01_asm, (uint32_t)exception_02_asm,
    (uint32_t)exception_03_asm, (uint32_t)exception_04_asm, (uint32_t)exception_05_asm,
    (uint32_t)exception_06_asm, (uint32_t)exception_07_asm, (uint32_t)exception_08_asm,
    (uint32_t)exception_09_asm, (uint32_t)exception_0A_asm, (uint32_t)exception_0B_asm,
    (uint32_t)exception_0C_asm, (uint32_t)exception_0D_asm, (uint32_t)exception_0E_asm,
    (uint32_t)exception_0F_asm, (uint32_t)exception_10_asm, (uint32_t)exception_11_asm,
    (uint32_t)exception_12_asm, (uint32_t)exception_13_asm, (uint32_t)exception_14_asm,
    (uint32_t)exception_15_asm, (uint32_t)exception_16_asm, (uint32_t)exception_17_asm,
    (uint32_t)exception_18_asm, (uint32_t)exception_19_asm, (uint32_t)exception_1A_asm,
    (uint32_t)exception_1B_asm, (uint32_t)exception_1C_asm, (uint32_t)exception_1D_asm,
    (uint32_t)exception_1E_asm, (uint32_t)exception_1F_asm
};



/* idt_init()
 * DESCRIPTION: Master function to completely initialize the IDT.
 *              Should be the only function the kernel itself calls,
 *              this will call everything else.
 */
void idt_init() {
    setup_idt();            // Create all NUM_VEC IDT descriptors
    idt_exception_setup();  // Setup the 32 Intel-interrupts
    // Initialize the system call vector
    set_idt_gate(SYSCALL_VECTOR, (uint32_t)syscall_handle, KERNEL_CS, GATE_SIZE_32, USER_PRIV, GATE_PRESENT, TRAP_GATE);
}

/* setup_idt()
 * DESCRIPTION: Fills the entire IDT with empty descriptors, even for Intel-interrupts
 * INPUTS: None
 * OUTPUTS: None
 * RETURN: None
 */
void setup_idt() {
    int i;
    for (i = 0; i < NUM_VEC; i++) {
        idt_desc_t new_desc;                        // Create the new descriptor
        new_desc.offset_31_16   = 0x0000;           //  
        new_desc.present        = GATE_NOT_PRESENT; // Say that there is currently nothing here
        new_desc.dpl            = USER_PRIV;        // Give the empty interrupts user-privelege
        new_desc.reserved0      = 0x0;              // 
        new_desc.size           = GATE_SIZE_32;     // Set the gates to be 32-bit
        new_desc.reserved1      = 0x1;              // Everything will be an interrupt gate by default 
        new_desc.reserved2      = 0x1;              // 
        new_desc.reserved3      = 0x0;              // 
        new_desc.reserved4      = 0x00;             // 
        new_desc.seg_selector   = 0x0000;           // 
        new_desc.offset_15_00   = 0x0000;           // 

        idt[i] = new_desc;
    }
}

/* idt_exception_setup()
 * DESCRIPTION: Sets up descriptors for all 32 Intel-interrupts 
 * INPUTS: None
 * Outputs: None
 * RETURN: None
 */
void idt_exception_setup() {
    int i;
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
        // Exceptions 3 and 4 utilze trap gates, so they must be set appropriately
        uint8_t gate_type = INTERRUPT_GATE;
        if (i == 3 || i == 4) { gate_type = TRAP_GATE; }

        // Set the gates
        set_idt_gate(i, interrupt_invocation[i], KERNEL_CS, GATE_SIZE_32, KERNEL_PRIV, GATE_PRESENT, gate_type);
    }
}

/* set_idt_gate()
 * DESCRIPTION: Sets the specified descriptor of the IDT to the values given
 * INPUTS: idt_idx - Index of the descriptor to be modified
 *         offset - Location of the assembly-level handler function for the interrupt
 *         segment - Segment selector to for the descriptor to be set to
 *         gate_size - 1-bit value determining if gate is 16 bits (0) or 32 (1)
 *         gate_dpl - Privelege level of the descriptor
 *         gate_presence - Whether the segment is present or not
 *         gate_type - What type of gate this should be set to
 * OUTPUTS: None
 * RETURN: None
 */
void set_idt_gate(int idt_idx, uint32_t offset, uint16_t segment, uint32_t gate_size, uint32_t gate_dpl, uint32_t gate_presence, uint32_t gate_type) {
    SET_IDT_ENTRY(idt[idt_idx], offset);
    idt[idt_idx].present    = gate_presence;
    idt[idt_idx].dpl        = gate_dpl;
    idt[idt_idx].reserved0  = 0x0;
    idt[idt_idx].size       = gate_size;
    switch(gate_type) {
        case (0x0): {
            idt[idt_idx].reserved1 = 0x1;
            idt[idt_idx].reserved2 = 0x0;
            idt[idt_idx].reserved3 = 0x1;
            break;
        }
        case (0x1): {
            idt[idt_idx].reserved1 = 0x1;
            idt[idt_idx].reserved2 = 0x1;
            idt[idt_idx].reserved3 = 0x0;
            break;
        }
        case (0x2): {
            idt[idt_idx].reserved1 = 0x1;
            idt[idt_idx].reserved2 = 0x1;
            idt[idt_idx].reserved3 = 0x1;
            break;
        }
    }
    idt[idt_idx].reserved4      = 0x00;
    idt[idt_idx].seg_selector   = segment;
}

/* get_interrupt_invoc()
 * DESCRIPTION: Returns a function pointer to the interrupt invocation in question
 * INPUTS: invoc - The interrupt who's invocation the user wants to retrieve
 * OUTPUTS: None
 * RETURN: The function pointer to the requested invocation
 */
uint32_t get_interrupt_invoc(uint32_t invoc) {
    return interrupt_invocation[invoc];
}


#endif
