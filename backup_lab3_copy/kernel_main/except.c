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
		shell_command(input_buffer);
		buffer_arrow=0;
		transmit_arrow=0;
		uart_send('#');
	}
	else transmit_arrow+=1;
	*AUX_MU_IER_REG &= ~(1<<1);
}
