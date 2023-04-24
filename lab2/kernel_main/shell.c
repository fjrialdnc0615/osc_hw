#include "utils.h"
#include "mailbox.h"
#include "_cpio.h"
#include "dtb.h"
#include "uart.h"
#include "allocator.h"
//from start.S extract _dtb_ptr
extern void *_dtb_ptr;
void shell() {
	//define space
	char array_space[128] = {0};
	

	while(1){
		uart_puts("# ");
		//define keyin 
		read_command(array_space);
		char* input_string = array_space;
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
		else
			uart_puts("no related command!\n");
		
	};
}

