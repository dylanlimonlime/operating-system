#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "exception.h"
#include "interrupt_invoc.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "rtc.h"
#include "terminal.h"
#include "filesys.h"
#include "syscall.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}



/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* exception_content_test
 * 
 * Asserts that the exceptions are all set correctly 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: idt_exception_setup, set_idt_gate 
 * Files: idt.h/c
 */

int exception_content_test() {
    TEST_HEADER;
    int i;
    int result = PASS;
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
        uint32_t offset = (idt[i].offset_31_16 << 16) + idt[i].offset_15_00;
        if (offset != get_interrupt_invoc(i)) {
            result = FAIL;
            printf("   interrupt invocation memory address: %X\n", get_interrupt_invoc(i));
            printf("   offset value of exception %X: %X\n", i, offset);
        }
        if (idt[i].seg_selector != KERNEL_CS) {
            result = FAIL;
            printf("   exception %X segment selector is not KERNEL_CS", i);
        }
        if (idt[i].present != 1) {
            result = FAIL;
            printf("   exception %X not set to present", i);
        }
        if (idt[i].dpl != 0) {
            result = FAIL;
            printf("   exception %X does not have kernel level privelege", i);
        }
    }

    return result;
}

/* Exception Tests
 * 
 * Tests various exceptions to make sure they work as expected
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: idt_exception_setup, set_idt_gate, exception.c
 * Files: idt.h/c, eXception.h/c
 */
int exception_00_test() {
    TEST_HEADER;
    int x = 5;
    int y = 0;
    int z = x / y;
    x = z;
    return FAIL; // If we get to this point, the test has failed
}
// WONT WORK
int exception_04_test() {
    TEST_HEADER;
    uint8_t x = 0xFF;
    x = x + 1;
    asm("INTO");
    return FAIL;
}

int exception_0B_test() {
    TEST_HEADER;
    char *str;
    str = "HEHE";
    *(str+1) = 'n';
    return FAIL; // If we get here, the test has failed
}

/*
void all_exception_test() {
    uint8_t i = 0x00;
    for (i = 0; i < NUM_EXCEPTIONS; i++) {
       asm volatile ("int %0" : "r"(i));
    }
}
*/

/* keyboard_ID_test
 * 
 * Makes sure that the keyboard descriptor is set properly
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: keyboard.c
 * Files: keyboard.h/c
 */
int keyboard_ID_test() {
    TEST_HEADER;
    int result = PASS;
    idt_desc_t keyboard_desc = idt[KEYBOARD_PIC_PORT];
    
    if (keyboard_desc.present != 0x1) { 
        result = FAIL;
        printf("   keyboard descriptor not set to present\n");
    }
    if (keyboard_desc.dpl != KERNEL_PRIV) {
        result = FAIL;
        printf("   keyboard doesn't have kernel-privlelge\n");
        printf("      priv = %X", keyboard_desc.dpl);
    }
    if (keyboard_desc.size != GATE_SIZE_32) {
        result = FAIL;
        printf("   descriptor size != 32)");
    }
    if (keyboard_desc.reserved1 != 0x1) {
        result = FAIL;
        printf("   reserved1 not set to 0x1");
    }
    if (keyboard_desc.reserved2 != 0x1) {
        result = FAIL;
        printf("   reserved2 not set to 0x1");
    }
    if (keyboard_desc.reserved3 != 0x0) {
        result = FAIL;
        printf("   reserved0 not set to 0x0");
    }
    if (keyboard_desc.seg_selector != KERNEL_CS) {
        result = FAIL;
        printf("   seg_selector != KERNEL_CS");
        printf("      Value: %X", keyboard_desc.seg_selector);
    }
    return result;
}

/* Derefencing Pointer Test
 *
 * dereferences pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: none
 * Coverage: Paging on a valid page
 * Files: x86_desc.h/S, paging.c/h
 */
int valid_paging_test(){
    TEST_HEADER;
    int result = FAIL;
	int a = TABLE_INSIDE;
	int * b = &a;
	if(*b == a){
		result = PASS;
	}
    return result;
}

/* Invalid Derefencing Pointer Test
 *
 * Dereferences invalid pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: causes page fault exception
 * Coverage: Paging on an invalid page
 * Files: x86_desc.h/S, paging.c/h
 */
int invalid_paging_test(){
	TEST_HEADER;
    int result = FAIL;
	int *a = INVALID_ADDR;
	int b;
	b = *a;
    return result;
}

/* null_paging_test
*
* Attempts to dereference a NULL ptr
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: Causes page fault exception
* Coverage: Paging on an invalid page
* Files: x86_desc.h/S, paging.c/h
*/
int null_paging_test()
{
    TEST_HEADER;
    int result = FAIL;
	int* invalid = NULL;
	int dereference = 0;
	dereference =  *invalid;
	return result;
}

/* video_memory_paging
*
* Attempts to access video memory
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: Paging on video memory
* Files: x86_desc.h/S, paging.c/h
*/
int video_memory_paging_test()
{
    TEST_HEADER;
    int result = FAIL;
	int* invalid;
	invalid = (int*) VIDEO_MEM_INSIDE;
    int dereference;
	dereference = *invalid;
    if(dereference){
        result = PASS;
    }
	return result;
}

// NOT WORKING
/* kernel_memory_paging_test
*
* Attempts to dereference pointer to kernel memory
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: causes page fault exception
* Coverage: Paging in kernel memory
* Files: x86_desc.h/S, paging.c/h
*/
int kernel_memory_paging_test()
{
    TEST_HEADER;
    int result = FAIL;
	int* invalid;
	int dereference;
	invalid = (int*) KERNEL_MEM_INSIDE;
	dereference = *invalid;
    return result;
}



/* syscall_test
 * 
 * Makes sure that the system call IDT descriptor is set properly
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: syscall.c
 * Files: syscall.h/c
 */
int syscall_test() {
    TEST_HEADER;
    asm("int $0x80");
    return PASS;
}

/* Checkpoint 2 tests */

/* power_2_test
 * 
 * Makes sure that the is_power_2 function works correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: is_power_2()
 * Files: lib.h/c
 */

int power_2_test() {
    TEST_HEADER;
    // Number of test values to run
    uint8_t num_tests = 23;
    // Inputs of tests
    uint32_t inputs[] = {0, 132, 41432, 5235, 6, 7536, 9999, // Random numbers that aren't powers of 2
                         0x0001, 0x0002, 0x0004, 0x0008,     // Powers of 2 from 2^0 - 2^32
                         0x0010, 0x0020, 0x0040, 0x0080,
                         0x0100, 0x0200, 0x0400, 0x0800,
                         0x1000, 0x2000, 0x4000, 0x8000 };
    // Expected outputs of tests
    uint32_t outputs[] = {-1, -1, -1, -1, -1, -1, -1, // Random numbers should fail
                            0, 1, 2, 3,             // Expected outputs of powers of 2
                            4, 5, 6, 7,
                            8, 9, 10, 11,
                            12, 13, 14, 15 };
    
    int result = PASS;
    uint8_t i;
    // Put every input into the function and make sure they match the output
    for (i = 0; i < num_tests; i++) {
        int8_t x = is_power_2(inputs[i]);
        if (x != outputs[i]) {
            result = FAIL;
            printf("   is_power_2(%x) expected result: %x\n", inputs[i], outputs[i]);
            printf("   actual result: %x\n", x);
        }
    }
    return result;
}

/* rtc_tests
 * 
 * Makes sure that the rtc driver functions work correctly
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: rtc.c
 * Files: rtc.h/c
 */
/*int rtc_test() {
    rtc_open();
    uint16_t i, j;
    int result = PASS;
    // Test all possible powers of 2, as the is_power_2 function prevents non-powers of 2
    for (i = 0; i < 32; i++) {
        // Start at 2^0, go all the way to 2^32
        uint32_t freq = 0x1 << i;
        rtc_frequency = freq;
        
        // Write current frequency being tested
        int32_t write_result = rtc_write();
        // Different procedures for if it should pass or fail
        if (RTC_MIN <= freq && freq <= RTC_USER_MAX) {
            if (write_result == -1) { 
                result = FAIL;
                printf("   Write for freq %x failed\n", freq);
                break;
            }
            // If it passed properly, print a ton of chars to make sure read works
            clear_all();
            // Do each frequency for 3 * frequency (i.e. 3 seconds)
            for (j = 0; j < 3 * freq; j++) {
                rtc_read();
                putc('F');
            }
        }
        else if (write_result != -1) {
            result = FAIL;
            printf("   Write for freq %x didn't fail\n", freq);
        }
    }
    return result;
}*/

/* void terminal_test()
 * DESCRITPTION: test function that reads up to 128 bytes from 
 * keyboard and writes it to screen 16 bytes at a time
 * INPUT: NONE
 * OUTPUT: NONE
 * SIDE EFFECTS: reads and writes the keyboard buffer to screen
 */
void terminal_test() {
    text_colour(LIGHT_CYAN, BLACK);
    uint8_t buf[1024];
    int32_t counter;

    while (1) {
        if (-1 == (counter = read(0,buf,1023))) {printf("FAILURE"); return;}
	printf("%d", counter);
	putc('\n');
        if (-1 == (counter = write(0,buf,counter))) {printf("FAILURE"); return;}
	printf("%d", counter);
	putc('\n');
    }
}	
/* ls_test
*
* Tests directory operations
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: Tests if file systems can list all files in directory
* Files: x86_desc.h/S, filesys.c
*/
int ls_test() {
    TEST_HEADER;
	uint32_t BUFSIZE = ARBITRARY_BUFFER_SIZE;
	uint32_t cnt = 0, i;
	uint8_t buf[BUFSIZE];
	printf("listing all files in directory:\n");
	while (0 != (cnt = directory_read(0, buf, BUFSIZE - 1))) {
		if (-1 == cnt) {
			printf("Error occured when listing files\n");
			return FAIL;
		}
		buf[cnt] = '\n';
		for (i = 0; i < cnt + 1; i ++) {
			putc(buf[i]);
		}

	}

	return PASS;

}

/* file_read operations
*
* Test file operations
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: None
* Coverage: Tests if file systems can read all text from buffer
* Files: x86_desc.h/S, filesys.c
*/
int file_read_test(){
    TEST_HEADER;
	int result = PASS;
	uint32_t i;
	int32_t bytesR;
	uint8_t buf[BIG_BUF_SIZE];
    file_open((uint8_t*)"frame1.txt");
    bytesR = file_read((uint32_t)"frame1.txt", buf, BIG_BUF_SIZE);
    printf("file_size: %d\n", (int32_t)bytesR);
    if(bytesR <= 0){
		printf("Failed to read data\n");
		return FAIL;
	}
	for(i = 0; i < bytesR; i++){
		printf("%c", buf[i]);
	}
    file_close((uint32_t)"frame1.txt");
    return result;
}

/* file_read_offset_test
*
* Reads from same file multiple times, keeping track of location
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: Prints contents to screen
* Coverage: Tests if file systems will use offset
* Files: x86_desc.h/S, filesys.c
*/
int file_read_offset_test(){
    TEST_HEADER;
	int result = PASS;
	int32_t bytesR;
	uint8_t buf[BIG_BUF_SIZE];
    uint8_t temp_buf[BIG_BUF_SIZE];
    file_open((uint8_t*)"frame1.txt");
    bytesR = file_read((uint32_t)"frame1.txt", temp_buf, MAGIC_NUMBER_OFFSET);
    bytesR = file_read((uint32_t)"frame1.txt", buf, MAGIC_NUMBER_OFFSET);
    if(bytesR <= 0){
		printf("Failed to read data\n");
		return FAIL;
	}
	terminal_write(0, buf, BIG_BUF_SIZE);
    file_close((uint32_t)"frame1.txt");
    return result;
}

/* read_from_txt_test
*
* Attempts to read from object file
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: prints contents of file to screen
* Coverage: Tests if file systems can read non text from files
* Files: x86_desc.h/S, filesys.c
*/
int read_from_non_txt_test(){
	TEST_HEADER;
	int result = PASS;
	int32_t bytesR;
	uint8_t buf[BIG_BUF_SIZE];
    file_open((uint8_t*)"hello");
	bytesR = file_read((uint32_t)"hello", buf, BIG_BUF_SIZE);
	if(bytesR <= 0){
		printf("Failed to read data\n");
		return FAIL;
	}
	terminal_write(0, buf, BIG_BUF_SIZE);
    file_close((uint32_t)"hello");
	return result;
}

/* read_from_large_file
*
* Attempts to read from file with name largers than 32 bytes
* Inputs: None
* Outputs: PASS/FAIL
* Side Effects: prints contents of file to screen
* Coverage: Tests how system handles names larger that 32 bytes
* Files: x86_desc.h/S, filesys.c
*/
int read_from_large_file(){
	TEST_HEADER;
	int result = PASS;
    int i;
	int32_t bytesR;
	uint8_t buf[BIG_BUF_SIZE];
    file_open((uint8_t*)"verylargetextwithverylongname.txt");
	bytesR = file_read((uint32_t)"verylargetextwithverylongname.txt", buf, BIG_BUF_SIZE);
	if(bytesR <= 0){
		printf("Failed to read data\n");
		return FAIL;
	}
	for(i = 0; i < bytesR; i++){
		printf("%c", buf[i]);
	}
    file_close((uint32_t)"verylargetextwithverylongname.txt");
	return result;
}

/* Checkpoint 3 tests */
 void s_test() {
    const char * cmd = "counter";
    execute((uint8_t*)(cmd));
}


/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
    //TEST_OUTPUT("exception_content_test", exception_content_test());
    //TEST_OUTPUT("valid_paging_test", valid_paging_test());
    //TEST_OUTPUT("keyboard_ID_test", keyboard_ID_test());
    //TEST_OUTPUT("syscall_test", syscall_test());
    //TEST_OUTPUT("invalid paging_test", invalid_paging_test());    // THROWS PAGEFAULT
    //TEST_OUTPUT("null_paging_test", null_paging_test());          // THROWS PAGEFAULT
    //TEST_OUTPUT("video_memory_paging_test", video_memory_paging_test());
    //TEST_OUTPUT("kernel_memory_paging_test", kernel_memory_paging_test());    //THOWS PAGEFAULT
    //exception_00_test();
    //exception_04_test();  // WONT WORK
    //exception_0B_test();
    
    //TEST_OUTPUT("power_2_test", power_2_test());
    //TEST_OUTPUT("rtc_test", rtc_test());

    //terminal_test();
    //while (1) if (ENTER) {buf_index = 0; ENTER = 0; break;} 
    //while statement that runs until enter is pressed. Runs twice since terminal_test calls enter itself
    //once. Running twice allows an enter keypress after the test to run the next test
    //while (1) if (ENTER) {buf_index = 0; ENTER = 0; break;}
       
    s_test();	
    //TEST_OUTPUT("list directory test", ls_test());
    //TEST_OUTPUT("file_read_test", file_read_test());
    //TEST_OUTPUT("file_read_offset_test", file_read_offset_test());
    //TEST_OUTPUT("read_from_non_txt_test", read_from_non_txt_test());
    //TEST_OUTPUT("read_from_large_file", read_from_large_file());
    return;
}

