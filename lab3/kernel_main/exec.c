#include "uart.h"
#include "_cpio.h"
#include "utils.h"
#include "allocator.h"

extern struct file* files;
extern int file_num;


void exec_command(char* args1){
	struct file* files_begin_ptr = files;
	//char* command_ptr = (char*)simple_malloc(32);
	unsigned check_value = 0; //for checking find out the file or not
	//read_command(command_ptr);
	
	for(int i=0;i<file_num;i++){
		if(str_compare(args1,(char *)(files->file_name))){
						
			
			char *target = (char *) 0x20000;
			for (int j = 0; j < files->file_size; j++)
				*target++ = files->file_content[j];
			
			unsigned long spsr_el1 = 0x340; //from 3c0 to 340(unmasked irq interrupt)
			asm volatile ("msr spsr_el1, %0" :: "r" (spsr_el1));
			unsigned long jump_addr = 0x20000;
			asm volatile ("msr elr_el1, %0":: "r" (jump_addr));
			unsigned long sp = (unsigned long)simple_malloc(32);
			asm volatile("msr sp_el0, %0":: "r" (sp));
			asm volatile("eret");

			check_value=1;
			break;
		}
		files++;
	}
	if(check_value==0)
		uart_puts("no file here!\n");
	files = files_begin_ptr;
	
}
