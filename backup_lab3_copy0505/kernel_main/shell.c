#include "utils.h"
#include "mailbox.h"
#include "_cpio.h"
#include "dtb.h"
#include "uart.h"
#include "allocator.h"
#include "exec.h"
#include "timer.h"
#define AUX_MU_IER_REG ((volatile unsigned int*)(0x3f215044))
extern unsigned int timer_begin;
extern void *_dtb_ptr;

char* args[5] = {0};

void shell_command(char* input_string) {
	//disable receiver interrupt!
	
	*AUX_MU_IER_REG &= !(1<<0);

	unsigned int args_num = 0;

	char* is_ptr = input_string;
	while(1){
		if(*is_ptr ==' '){
			*is_ptr = '\0';
			args[args_num++] = input_string;
			is_ptr++;
			input_string = is_ptr;
		}
		else if(*is_ptr =='\0'){
			*is_ptr = '\0';
			args[args_num++] = input_string;
			is_ptr++;
			input_string = is_ptr;
			break;
		}
		else is_ptr++;
	}

	//define keyin 
	if(str_compare(args[0],"help"))
	{
		uart_puts("help	:print this help menu\n");
		uart_puts("hello	:print the hello menu\n");
		uart_puts("reboot	:reboot the device\n");
		uart_puts("info	:get the hardware information\n");
	}
	else if(str_compare(args[0],"hello"))
	{
		uart_puts("Hello World!\n");
	}
	else if (str_compare(args[0],"info"))
	{
		get_board_revision();
		get_size_and_memory_address();
		uart_puts("\n");
	}
	else if (str_compare(args[0],"reboot"))
		reset(500);
	else if (str_compare(args[0],"ls"))
	{	
		ls_command();
		uart_send('\r');
		uart_send('\n');
	}
	else if (str_compare(args[0],"cat"))
	{
		cat_command(args[1]);
		uart_send('\r');
		uart_send('\n');
	}
	else if (str_compare(args[0],"dtb"))
	{
		fdt_traverse(print_dtb,_dtb_ptr);
	}
	else if (str_compare(args[0],"malloc"))
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
		uart_send('\n');

		uart_puts("test_char* 2:");
		uart_puts(test_malloc2);
		uart_send('\n');
	}
	else if (str_compare(args[0],"exec"))
	{
		exec_command(args[1]);
	}
	//enable or disable timer
	else if (str_compare(args[0],"timer"))
	{
		unsigned long long CNTP_CTL_EL0;
		asm volatile ("mrs %0, CNTP_CTL_EL0":"=r"(CNTP_CTL_EL0));
		if((CNTP_CTL_EL0&1<<0)!=0){
			uart_puts("disable timer\n");
			asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(0));
		}
		else{
			uart_puts("enable timer\n");
			asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(1));
		}
	}
	else if (str_compare(args[0],"settimeout"))
	{
		
		timer_begin=0;
		set_timeout(args[1],args[2]);
	}
	else if (str_compare(args[0],"async")){
		enable_uart_irt();
		while(1) asm volatile("nop");
	}
	else
		uart_puts("no related command!\n");
		
	*AUX_MU_IER_REG |= (1<<0);
}

