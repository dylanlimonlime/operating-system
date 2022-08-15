#include "pit.h"

// global variable for pit frequency
uint16_t frequency = 0;

/* 
 * FUNCTION NAME: void pit_init()
 * DESCRIPTION:   initializes pit and enables interrupts
 * INPUTS:        none
 * OUTPUTS:       none
 * RETURN:        none
 */
void pit_init() {

    // set interrupt gate
    set_idt_gate(PIT_INTERRUPT_LOCATION, (uint32_t)pit_handle, KERNEL_CS, GATE_SIZE_32, KERNEL_PRIV, GATE_PRESENT, INTERRUPT_GATE);

    // generate interrupt
    outb(SQUARE_WAVE, MODE_REGISTER);

    // calculate pit frequency
    frequency = INPUT_SIGNAL / RELOAD_VALUE;

    // mask value before sending to port
    outb(frequency & MASK, CHANNEL_0);

    // right shift to get upper 8 bits
    outb(frequency >> EIGHT, CHANNEL_0);

    // enable irq 0
    enable_irq(IRQ_0);

}

/* 
 * FUNCTION NAME: void pit_interrupt_handle()
 * DESCRIPTION:   handler for when a pit interrupt comes in
 * INPUTS:        none
 * OUTPUTS:       none
 * RETURN:        none
 */ 
void pit_interrupt_handle() {
    scheduler();
    // send end of interrupt signal
    send_eoi(IRQ_0);

}

