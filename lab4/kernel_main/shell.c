#include "poly.h"

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
		uart_puts("ls	:show the ram filesystem\n");
		uart_puts("cat	:show the file content\n");
		uart_puts("dtb	:show dtb information\n");
		uart_puts("malloc	:demonstrate simple malloc\n");
		uart_puts("exec *file name*	:run file\n");
		uart_puts("settimeout *message* *duration*	:set time and get message\n");
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
		char *test_malloc = y_malloc(sizeof("12345"));
		test_malloc[0] = 'a';
		test_malloc[1] = 'b';
		test_malloc[2] = 'c';
		test_malloc[3] = '\0';
		char *test_malloc2 = y_malloc(sizeof("123"));
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
	else if (str_compare(args[0],"settimeout"))
	{
		
		set_timeout(args[1],args[2]);
	}
	else
		uart_puts("no related command!\n");
		
	*AUX_MU_IER_REG |= (1<<0);
}

