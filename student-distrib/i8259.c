/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define FULL_PIC_MASK       0xFF
#define MASTER_PIC_DATA     0x21
#define SLAVE_PIC_DATA      0xA1
#define ICW1_INIT           0x11
#define ICW2_MASTER_INIT    0x20
#define ICW2_SLAVE_INIT     0x28
#define ICW3_MASTER_INIT    0x04    // there is a slave at IR2, so the 3rd bit is a 1
#define ICW3_SLAVE_INIT     0x02    // the slave is connected at IR2
#define ICW4_INIT           0x01    // EOI
#define MASTER_PIC_BASE     0x20
#define SLAVE_PIC_BASE      0xA0

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init(void)
*  Description: This functions initializes the PIC
*  Inputs: NONE
*  Outputs: NONE
*  Side Effects: Initializes the PIC
*/
void i8259_init(void) {
    //unsigned char Master_save;
    //unsigned char Slave_save;

    unsigned long flags;
    cli_and_save(flags);

    //Master_save = inb(MASTER_PIC_DATA);
    //Slave_save = inb(SLAVE_PIC_DATA);

    //outb(FULL_PIC_MASK, MASTER_PIC_DATA); //masks the Master PIC
    //outb(FULL_PIC_MASK, SLAVE_PIC_DATA); //masks the Slave PIC
    
    //normally Id use outb_p, but for some reaason it wont compile
    outb(ICW1_INIT,MASTER_PIC_BASE); //ICW1 for Master
    outb(ICW2_MASTER_INIT, MASTER_PIC_DATA); //ICW2 for Master
    outb(ICW3_MASTER_INIT, MASTER_PIC_DATA); //ICW3 for Master
    outb(ICW4_INIT, MASTER_PIC_DATA); //ICW4 for Master

    outb(ICW1_INIT,SLAVE_PIC_BASE); //ICW1 for Slave
    outb(ICW2_SLAVE_INIT, SLAVE_PIC_DATA); //ICW2 for Slave
    outb(ICW3_SLAVE_INIT, SLAVE_PIC_DATA); //ICW3 for Slave
    outb(ICW4_INIT, SLAVE_PIC_DATA); //ICW4 for Slave


	outb(FULL_PIC_MASK, MASTER_PIC_DATA);
	outb(FULL_PIC_MASK, SLAVE_PIC_DATA);
    //outb(Master_save, MASTER_PIC_DATA); 
    //outb(Slave_save, SLAVE_PIC_DATA);

	enable_irq(ICW3_SLAVE_INIT); //the slave always has to be enabled

    restore_flags(flags);
}

/* disable_irq(uint32_t irq_num)
*  Description: masks the interrupt at the specified iRQ number
*  Inputs: irq_num which holds the irq number of the PIC
*  Outputs: NONE
*  Side Effects: masks the pic at the interrupt number
*/
void disable_irq(uint32_t irq_num) {
    unsigned char value;
    unsigned long port;
    unsigned long flags;
    cli_and_save(flags);

    if (irq_num <= 7 && irq_num >= 0) { //master irq_num must be within 0 and 7 inclusive according to definition at top
        port = MASTER_PIC_DATA; //sets port to master 
        value = inb(port) | (1 << irq_num);
	}
    else if (irq_num <= 15 && irq_num >= 8) { //slave irq_num must be within 8 and 15 inclusive according to definition at top
        port =  SLAVE_PIC_DATA; //sets port to slave
        value = inb(port) | (1 << (irq_num - 8)); //subtract by 8 since slave IRQ goes from 8 to 15. subtrating by eight gives irq port in PIC
	}

    outb(value, port);

    restore_flags(flags);
}

/* enable_irq(uint32_t irq_num)
*  Description: unmasks the interrupt at the specified iRQ number
*  Inputs: irq_num which holds the irq number of the PIC
*  Outputs: NONE
*  Side Effects: unmasks the pic at the interrupt number
*/
void enable_irq(uint32_t irq_num) {
    unsigned char value;
    unsigned long port;
    unsigned long flags;
    cli_and_save(flags);

    if (irq_num <= 7 && irq_num >= 0) { //master irq_num must be within 0 and 7 inclusive according to definition at top
        port = MASTER_PIC_DATA; //sets port to master 
        value = inb(port) & ~(1 << irq_num);
	}
    else if (irq_num <= 15 && irq_num >= 8) { //slave irq_num must be within 8 and 15 inclusive according to definition at top
        port =  SLAVE_PIC_DATA; //sets port to slave
        value = inb(port) & ~(1 << (irq_num - 8)); //subtract by 8 since slave IRQ goes from 8 to 15. subtrating by eight gives irq port in PIC
	}

    outb(value, port); //writes the value to the port

    restore_flags(flags); //returns flags
}

/* send_eoi(uint32_t irq_num)
*  Description: sends an EOI to the interrupt number on the PIC
*  Inputs: irq_num which holds the IRQ number of the PIC
*  Outputs: NONE
*  Side Effects: ends interrupt requests on the IRQ of the PIC
*/
void send_eoi(uint32_t irq_num) {
    unsigned long value;
    unsigned long port;
    unsigned long flags;
    cli_and_save(flags);

    if (irq_num <= 7 && irq_num >= 0) { //master irq_num must be within 0 and 7 inclusive according to definition at top
        port = MASTER_PIC_BASE; //sets port to master 
        value = EOI | irq_num;
	}
    else if (irq_num <= 15 && irq_num >= 8) { //slave irq_num must be within 8 and 15 inclusive according to definition at top
        port =  SLAVE_PIC_BASE; //sets port to slave
        value = EOI | (irq_num - 8);
        outb(value, port); //you must OUTB since the slave has to send EOI to both master and slave
        port = MASTER_PIC_BASE;
        value = EOI | ICW3_SLAVE_INIT;
	}

    outb(value, port); //writes the value to the port

    restore_flags(flags); //returns flags
}

