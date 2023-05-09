#include "utils.h"
#include "mailbox.h"
#include "_cpio.h"
#include "dtb.h"
#include "uart.h"
#include "allocator.h"
#include "exec.h"

#define AUX_MU_IER_REG ((volatile unsigned int*)(0x3f215044))

extern void *_dtb_ptr;

void shell_command(char* input_string) {
	//disable receiver interrupt!
	
	*AUX_MU_IER_REG &= !(1<<0);

	//define keyin 
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
		uart_send('\n');

		uart_puts("test_char* 2:");
		uart_puts(test_malloc2);
		uart_send('\n');
	}
	else if (str_compare(input_string,"exec"))
	{
		exec_command();
	}
	//enable or disable timer
	else if (str_compare(input_string,"timer"))
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
	else if (str_compare(input_string,"async")){
		enable_uart_irt();
		while(1) asm volatile("nop");
	}
	else
		uart_puts("no related command!\n");
		
	*AUX_MU_IER_REG |= (1<<0);
	//};
}

