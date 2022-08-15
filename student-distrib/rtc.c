#ifndef _RTC_C
#define _RTC_C


#include "x86_desc.h"
#include "idt.h"
#include "i8259.h" 
#include "lib.h"
#include "interrupt_invoc.h"
#include "rtc.h"


/* rtc_init()
 * DESCRIPTION: Initializes the RTC and enables interrupts.
 *              Leaves frequency at default.
 * INPUTS: None
 * OUTPUTS: None
 * RETURN: None
 */
void rtc_init() {
    // Start off with a slow rate
    rtc_set_rate(0x0F);

    // Set up the gate beforehand so things don't get whack
    set_idt_gate(RTC_INTERRUPT_LOCATION, (uint32_t)rtc_handle, KERNEL_CS, GATE_SIZE_32, KERNEL_PRIV, GATE_PRESENT, INTERRUPT_GATE);
    
    // Enable interrupts on the RTC itself
    outb(RTC_REGISTER_B, RTC_INDEX_PORT);   
    uint8_t prev_reg = inb(RTC_DATA_PORT);  // Save the previous contents of register B
    outb(RTC_REGISTER_B, RTC_INDEX_PORT);
    outb((prev_reg | RTC_ENABLE_INTERRUPT), RTC_DATA_PORT);
    
    // Unmask the interrupts on the IDT
    enable_irq(RTC_PIC_PORT);
    return;
}

/* rtc_set_rate()
 * DESCRIPTION: Sets the rate at which the RTC will send an interrupt
 * INPUTS: rate - Value from 3-15. The frequency will be right shifted by this value minus 1
 * OUTPUTS: None
 * RETURN: None
 */
void rtc_set_rate(uint8_t rate) {
    rate &= 0x0F;       // Rate cannot be greater than 15    
    if (rate >= 0x3) {  // Rate also cannot be less than 3
        outb(RTC_REGISTER_A, RTC_INDEX_PORT);

        // Store the previous state of the register; upper 4 bits are control bits
        uint8_t prev_reg = inb(RTC_DATA_PORT);
        outb(RTC_REGISTER_A, RTC_INDEX_PORT);

        // Write the new rate while preserving the control bits
        outb((prev_reg & 0xF0) | rate, RTC_DATA_PORT);
    }
    return;
}

/* rtc_enable_irq()
 * DESCRIPTION: Enables RTC interrupts
 * INPUTS: None
 * OUTPUTS: None
 * RETURN: None
 */
void rtc_enable_irq() {
    enable_irq(RTC_PIC_PORT);
}

/* rtc_disable_irq()
 * DESCRIPTION: Disables RTC interrupys
 * INPUTS: None
 * OUTPUTS: None
 * RETURN: None
 */
void rtc_disable_irq() {
    disable_irq(RTC_PIC_PORT);
}

/* rtc_interrupt_handle()
 * DESCRIPTION: C-level handler for when an RTC interrupt comes in
 * INPUTS:  None
 * OUTPUTS: None
 * RETURN:  None
 */ 
void rtc_interrupt_handle() {
    // Read and throw away whatever the RTC is sending as it's useless
    outb(RTC_REGISTER_C, RTC_INDEX_PORT);
    inb(RTC_DATA_PORT);

    // Clears flag so rtc_read can break if it's running
    RTC_read_flag = 0x00;
    
    // Unmask the interrupts
    send_eoi(RTC_PIC_PORT);
    return;
}

/* rtc_open()
 * DESCRIPTION: Opens file descriptor for RTC (eventually
 *              Sets rate to 2 HZ
 * INPUTS:  None
 * OUTPUTS: None
 * RETURN:  0 on success, which is always.
 */
int32_t rtc_open(const uint8_t* filename) {
    // Set RTC rate to 2 Hz by default
    enable_irq(RTC_PIC_PORT);
    rtc_set_rate(RTC_2_HZ);
    return 0;
}

/* rtc_read()
 * DESCRIPTION: Holds program until RTC interrupt comes in
 * INPUTS:  None
 * OUTPUTS: None
 * RETURN:  0 on success (which is always unless interrupts are masked)
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    RTC_read_flag = 0x01;   // Sets flag to watch
    while (RTC_read_flag == 0x01) { continue; } // Holds program until flag is unset by interrupt handler
    return 0;
}

/* rtc_write()
 * DESCRIPTION: Sets the RTC Frequency
 * INPUTS:  None
 * OUTPUTS: 0 on success, -1 on failure
 * RETURN:  None
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    int32_t result;
    int32_t freq = *((int32_t*)(buf));
    // Make sure desired frequency is within range
    if (freq < RTC_MIN || freq > RTC_USER_MAX) { result = -1; }
    else {
        int8_t rate = 16 - is_power_2(freq);
        if (rate >= 17) { result = -1; } // Return failure if rtc_frequency isn't a power of 2
        else { 
            rtc_set_rate(rate);
            result = 0;
        }
    }
    return result;
}


/* rtc_close()
 * DESCRIPTION: Closes file descriptor (eventually)
 * INPUTS: None
 * OUTPUTS: None
 * RETURN: 0 on success, which is always
 *
 */
int32_t rtc_close(int32_t fd) {
    return 0; // Nothing for now, as system calls have yet to be implemented.
}

#endif
