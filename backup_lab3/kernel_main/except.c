#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
#include "_cpio.h"
#include "except.h"
#define AUX_MU_IO       ((volatile unsigned int*)(0x3f215040))
#define AUX_MU_IER_REG ((volatile unsigned int*)(0x3f215044))

void timer_handler(){
	unsigned long long cntpct_el0;
	asm volatile ("mrs %0, cntpct_el0":"=r"(cntpct_el0));
	unsigned long long cntfrq_el0;
	asm volatile ("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
	unsigned long long time = cntpct_el0/cntfrq_el0;
	uart_hex(time);
	/*
	if(time_check!=time){
		uart_hex(time);
		time_check = time;
	}
	*/
	asm volatile ("msr cntp_tval_el0, %0"::"r"(2*cntfrq_el0));
}
void receiver_handler(){
    char r=(char)(*AUX_MU_IO);
	if(r=='\r') r='\0';
	input_buffer[buffer_arrow++] = r;
	
	if(buffer_arrow!=transmit_arrow){
		//enable async transmit handler
		*AUX_MU_IER_REG |= (1<<1);
	}
}

void transmit_handler(){
	uart_send(input_buffer[transmit_arrow]);
	if(input_buffer[transmit_arrow]=='\0'){
		uart_send('\n');






		if(str_compare(input_buffer,"help"))
		{
			uart_puts("help	:print this help menu\n");
			uart_puts("hello	:print the hello menu\n");
			uart_puts("reboot	:reboot the device\n");
			uart_puts("info	:get the hardware information\n");
		}/*
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
		else if (str_compare(input_string,"ls"))
		{	
			ls_command();
			uart_send('\r');
			uart_send('\n');
		}
		else if (str_compare(input_string,"cat"))
		{
			cat_command();
			uart_send('\r');
			uart_send('\n');
		}
		else if (str_compare(input_string,"dtb"))
		{
			fdt_traverse(print_dtb,_dtb_ptr);
		}
		else if (str_compare(input_string,"malloc"))
		{
			char *test_malloc = simple_malloc(sizeof("12345"));
			test_malloc[0] = 'a';
			test_malloc[1] = 'b';
			test_malloc[2] = 'c';
			test_malloc[3] = '\0';
			char *test_malloc2 = simple_malloc(sizeof("123"));
			test_malloc2[0] = '1';
			test_malloc2[1] = '2';
			test_malloc2[2] = '3';
			test_malloc2[3] = '\0';
			
			uart_puts("test_char* 1:");
			uart_puts(test_malloc);
			*/














		buffer_arrow=0;
		transmit_arrow=0;
	}
	else transmit_arrow+=1;
	*AUX_MU_IER_REG &= ~(1<<1);
}
