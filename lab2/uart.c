#include "gpio.h"
#include "utils.h"
#include "mailbox.h"
/* Auxilary mini UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile unsigned int*)(MMIO_BASE+0x00215068))


//for reboot address
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    //since we want Uart1 receiver(gpio15) and transmitter(gpio14), they are located on the second group of 10 GPIO pins (pins 10-19) every GPFSEL control 10 bits
    r=*GPFSEL1;
    //following two lines are to set gpio14 and 15 to 010(which is alt5) which can activate transmitter and receiver
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
    
    //we already use alt5(txd1 and rxd1) so we need to disable basic input-output(disable pull up and pull down)
    //00 = Off – disable pull-up/down (Write to GPPUD to set the required control signal)
    *GPPUD = 0;            // enable pins 14 and 15
    //Wait 150 cycles – this provides the required set-up time for the control signal
    r=150; while(r--) { asm volatile("nop"); }
    //GPPUDCLK0 controls gpio 0~31 and GPPUDCLK1 controls 32~53
    //Set the control signals for the pull-up/down resistors for GPIO14 and GPIO15 to "on", which enables the internal pull-down resistor.
    *GPPUDCLK0 = (1<<14)|(1<<15);
    //Wait 150 cycles – this provides the required hold time for the control signal
    r=150; while(r--) { asm volatile("nop"); }
    //Write to GPPUDCLK0/1 to remove the clock 
    *GPPUDCLK0 = 0;        // flush GPIO setup

    r=500; while(r--) { asm volatile("nop"); }


    /* initialize UART */
    // if set the mini UART is enable.
    *AUX_ENABLE |=1;       
    // disable transmitter and receiver during configurataion. Prevent from data exchange during initialization.
    *AUX_MU_CNTL = 0;      
	// disable ier because currently you dont need interrupt.
	*AUX_MU_IER = 0;
    // set 11 so that using 8 bits which can be ascii and utf-8
    *AUX_MU_LCR = 3;
    // for basic serial communication(not too smart)       
    *AUX_MU_MCR = 0;
    // AUX_MU_BAUD register to 270 sets the UART baud rate to approximately 115200 bits
    *AUX_MU_BAUD = 270;    // 115200 baud
    //c6 = 11000110, 6,7 no FIFO and 1,2 clear transmit and receive. sacrifice reliability(buffer) to get low latency.
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    //UX_MU_CNTL register can then be set to 3 to enable both the transmitter and receiver
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
    /* map UART1 to GPIO pins */
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    //do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    //if fifth bit is 1(transmit FIFO can accept at least one byte) with not it will return false then break while.
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20));
    /* write the character to the buffer */
    //communicate with minicom so that minicom will print to screen.
    *AUX_MU_IO=c;
    
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    //This bit is set if the receive FIFO holds at least 1 symbol(bit 0). 
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */
    return r=='\r'?'\n':r;
}

/**
 * Display a string
 */
void uart_puts(char* s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_send('\r');
        uart_send(*s++);
    }
}

void uart_hex(unsigned int n)
{
	//setting char array
	int size_pointer = 33;
	char binary_container[size_pointer+1];
	binary_container[size_pointer--] = '\0';
	binary_container[size_pointer--] = '\n';
	while(size_pointer>0)
	{
		if(n%2 == 1)
                binary_container[size_pointer--]='1';
                else
                binary_container[size_pointer--]='0';
                n/=2;
	}
	
	char hex_dict[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e'};
	size_pointer = 0;
    	char hex_container[10];
    	int temp_num = 0;
        
    	for(int i=0;i<33;i++)
    	{
        	if(i%4==0 && i!=0)
            	{
               		hex_container[size_pointer++]=hex_dict[temp_num];
                	temp_num = 0;
            	};

            	if (binary_container[i]=='1')
                	temp_num += 1<<(3-(i%4)); //3, 2, 1, 0
    	}
	hex_container[size_pointer++] = '\n';
	hex_container[size_pointer] = '\0';
	uart_puts("0x");
	uart_puts(hex_container);
	
}

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

void shell() {
	//define space
	char array_space[128] = {0};
	//a pointer point to space
	char* input_string = array_space;
	char single_element;
	while(1)
	{
		
		uart_puts("# ");
		//define keyin 
		while(1){
			single_element = uart_getc();
			if(single_element == '\0') uart_puts("there is a zero\n");
			*input_string++ = single_element;
			uart_send(single_element);
			if(single_element == '\n'){
				*input_string = '\0';
				break;
			};
		};
		
		input_string = array_space;
			
		if(str_compare(input_string,"help"))
		{
			uart_puts("help	:print this help menu\n");
			uart_puts("hello	:print the hello menu\n");
			uart_puts("reboot	:reboot the device\n");
			uart_puts("info	:get the hardware information\n");
		}
		else if(str_compare(input_string,"hello"))
		{
			uart_puts("Hello World!\n");
		}
		else if (str_compare(input_string,"info"))
		{
			get_board_revision();
			get_size_and_memory_address();
			uart_puts("\n");
		}
		else if (str_compare(input_string,"reboot"))
			reset(500);
		
		else
			uart_puts("no related command!\n");
		
	};
}

