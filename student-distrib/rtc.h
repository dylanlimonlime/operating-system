#ifndef _RTC_H
#define _RTC_H

#define RTC_PIC_PORT 0x08
#define RTC_INTERRUPT_LOCATION 0x28

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71

#define RTC_REGISTER_A 0x0A
#define RTC_REGISTER_B 0x0B
#define RTC_REGISTER_C 0x0C

#define RTC_ENABLE_INTERRUPT 0x40
#define RTC_DISABLE_NMI 0x80

#define RTC_ACTUAL_FREQ 0x8000
#define RTC_MIN         0x2
#define RTC_USER_MAX    0x0400

#define RTC_2_HZ        0x0F


int8_t RTC_FLAG;

/* Flag for rtc_read */
int8_t RTC_read_flag;

/* Buffer for rtc_frequency */
uint32_t rtc_frequency;

/* Initializes the RTC */
extern void rtc_init();

/* Extern for now, change later */
extern void rtc_set_rate(uint8_t rate);

/* Function for later use when virtualizing */
extern void rtc_set_virtual_rate(uint8_t prog_num, uint8_t rate);

/* Enable interrupts from RTC */
extern void rtc_enable_irq();

/* Disable interrupts from RTC */
extern void rtc_disable_irq();

/* Handler for RTC interrupts */
extern void rtc_interrupt_handle();

/* Opens file descriptor (Eventually) */
extern int32_t rtc_open(const uint8_t* filename);

/* Holds function until RTC interrupt comes in */
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Changes RTC frequency to whatever is in rtc_frequency */
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Closes file descriptor (eventually) */
extern int32_t rtc_close(int32_t fd);

#endif
