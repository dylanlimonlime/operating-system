/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "idt.h"
#include "keyboard.h"
#include "rtc.h"
#include "paging.h"
#include "syscall.h"
#include "filesys.h"
#include "multi_term.h"
#include "scheduler.h"
#include "pit.h"

#define RUN_TESTS

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {
    multiboot_info_t *mbi;
    uint32_t boot_block_addr;

    /* Clear the screen. */
    clear();
    ATTRIB = 0x7;
    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        boot_block_addr = (uint32_t)mod->mod_start;
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }
    
    cur_scheduled_terminal = 0;
	cur_terminal = 0;
    terminal_process_nums[0] = NOT_ASSIGNED;
    terminal_process_nums[1] = NOT_ASSIGNED;
    terminal_process_nums[2] = NOT_ASSIGNED;

    /* Init the PIC */
    i8259_init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    
    /* Init the IDT */
    idt_init();
    
    /* Initialize the keyboard */
    keyboard_init();

	/* Initialize the RTC */
	rtc_init();

    /* Enable paging */
	paging_init();

    /* Initialize filesys */
    filesys_init(boot_block_addr);

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    sti();
    video_mem = (char *)VIDEO;
	int32_t j;
	int32_t shade;
    int32_t lol;
	for (j = 0; j < NUM_COLS*NUM_ROWS; j++) {
	    lol = 1*FOUR_KB;
	    shade = BLACK + (WHITE << BACKGROUND); //allows a white background with a black text
		*(uint8_t *)(video_mem + lol + (j << 1)) = ' ';
		*(uint8_t *)(video_mem + lol + (j << 1) + 1) = shade;
		
		lol = 2*FOUR_KB;
        shade = LIGHT_RED + (DARK_GRAY << BACKGROUND); //allows a dark_gray background with a light_red text
        *(uint8_t *)(video_mem + lol + (j << 1)) = ' ';
		*(uint8_t *)(video_mem + lol + (j << 1) + 1) = shade;
	
		lol = 3*FOUR_KB;
		shade = LIGHT_GREEN + (LIGHT_BLUE << BACKGROUND); //allows a light_blue background with a light_green text
		*(uint8_t *)(video_mem + lol + (j << 1)) = ' ';
		*(uint8_t *)(video_mem + lol + (j << 1) + 1) = shade;
	}
    sscreeny[0] = 0;
    sscreeny[1] = 0;
    sscreeny[2] = 0;
    sscreenx[0] = 0;
    sscreenx[1] = 0;
    sscreenx[2] = 0;
    
	clear_all();
	text_colour(WHITE, BLACK);
	printf("                                                                    Sanjay Parhi");
	printf("                                                                   Fawaz Tirmizi");
	printf("                                                                   Nithin Nathan");
	printf("                                                                       Dylan Lim");
	CURSOR = 1;
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nPress ENTER to clear screen and start shell");
	CURSOR = 0; //FLAG tells cursor not to change
	set_cursor(0,0); //moves cursor to beginning of screen
	int8_t colour1 = LIGHT_BLUE;
    int rate = 4; //RTC-rate to prevent tearing

    text_colour(LIGHT_MAGENTA, BLACK);
    printf("                      _________  ___  ___  _______ \n");
    printf("                     |\\___   ___\\\\  \\|\\  \\|\\  ___ \\\n");
    printf("                     \\|___ \\  \\_\\ \\  \\\\\\  \\ \\   __/|\n");
    printf("                          \\ \\  \\ \\ \\   __  \\ \\  \\_|/__\n");
    printf("                           \\ \\  \\ \\ \\  \\ \\  \\ \\  \\_|\\ \\    \n"); 
    printf("                            \\ \\__\\ \\ \\__\\ \\__\\ \\_______\\     \n"); 
    printf("                             \\|__|  \\|__|\\|__|\\|_______|           \n"); 
	printf("                                                         \n");
    text_colour(YELLOW, BLACK);
    printf("         ___  __    _______   ________  ________   _______   ___     \n");   
    printf("        |\\  \\|\\  \\ |\\  ___ \\ |\\   __  \\|\\   ___  \\|\\  ___ \\ |\\  \\     \n");   
    printf("        \\ \\  \\/  /|\\ \\   __/|\\ \\  \\|\\  \\ \\  \\\\ \\  \\ \\   __/|\\ \\  \\  \n");  
    printf("         \\ \\   ___  \\ \\  \\_|/_\\ \\   _  _\\ \\  \\\\ \\  \\ \\  \\_|/_\\ \\  \\    \n");  
    printf("          \\ \\  \\\\ \\  \\ \\  \\_|\\ \\ \\  \\\\  \\\\ \\  \\\\ \\  \\ \\  \\_|\\ \\ \\  \\____ \n"); 
    printf("           \\ \\__\\\\ \\__\\ \\_______\\ \\__\\\\ _\\\\ \\__\\\\ \\__\\ \\_______\\ \\_______\\   \n"); 
	printf("            \\|__| \\|__|\\|_______|\\|__|\\|__|\\|__| \\|__|\\|_______|\\|_______|                \n"); 
    
	while (1) {
		if (enter[cur_terminal]) {buf_index[cur_terminal] = 0; enter[cur_terminal] = 0; break;} //checks for enter keypress
	    else {			
            set_cursor(0,0); //sets cursor to location of KRASHER word
	        set_cursor(0,16);
	        text_colour(colour1, BLACK);
            printf("   ___  __    ________  ________  ________  ___  ___  _______   ________          |\\  \\|\\  \\ |\\   __  \\|\\   __  \\|\\   ____\\|\\  \\|\\  \\|\\  ___ \\ |\\   __  \\         \\ \\  \\/  /|\\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\___|\\ \\  \\\\\\  \\ \\   __/|\\ \\  \\|\\  \\         \\ \\   ___  \\ \\   _  _\\ \\   __  \\ \\_____  \\ \\   __  \\ \\  \\_|/_\\ \\   _  _\\         \\ \\  \\\\ \\  \\ \\  \\\\  \\\\ \\  \\ \\  \\|____|\\  \\ \\  \\ \\  \\ \\  \\_|\\ \\ \\  \\\\  \\|         \\ \\__\\\\ \\__\\ \\__\\\\ _\\\\ \\__\\ \\__\\____\\_\\  \\ \\__\\ \\__\\ \\_______\\ \\__\\\\ _\\          \\|__| \\|__|\\|__|\\|__|\\|__|\\|__|\\_________\\|__|\\|__|\\|_______|\\|__|\\|__|                                       \\|_________|                                ");   
	    }
	    colour1 = ((colour1 + 1) % NUM_OF_COLOURS) + 1;
	    rtc_read(0,0,0); //sets timing on RTC
	    rtc_write(0,&rate,4);
    }
    
	rtc_disable_irq();
    TERMINAL_FLAG = 1;
	CURSOR = 1; //allows CURSOR flag to be changed
    
	text_colour(BLACK, WHITE);
	//text_colour(WHITE, BLACK);
	clear_all();
#ifdef RUN_TESTS
    /* Run tests */
    //launch_tests();
#endif
    
    cur_scheduled_terminal = 0xFF;
    /* Execute the first program ("shell") ... */
	/* Initialize pit */
    pit_init();
	
	const char* cmd = "shell";
	execute((uint8_t*)(cmd));
    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
