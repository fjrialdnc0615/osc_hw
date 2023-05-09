#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
#include "_cpio.h"
#include "except.h"
#include "timer.h"
#define AUX_MU_IO	   ((volatile unsigned int*)(0x3f215040))
#define AUX_MU_IER_REG ((volatile unsigned int*)(0x3f215044))

void timer_handler(){
	
    unsigned long long cntpct_el0;
    asm volatile ("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    unsigned long long cntfrq_el0;
    asm volatile ("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    
    //execute current timer
    uart_puts("\nMessage:");
    timeout_event_head->callback(timeout_event_head->msg);
    
    unsigned long long current_time = cntpct_el0/cntfrq_el0;
    
    uart_puts("\nCurrent time:");
    uart_hex(current_time);
    uart_puts("\nDuration time:");
    uart_hex(timeout_event_head->register_time);
    //setting next timer
	uart_puts(timeout_event_head->next->msg);
    struct timeout_event* next = timeout_event_head->next;
    if(next!=NULL){
        asm volatile ("msr cntp_cval_el0, %0"::"r"(next->exe_time));
    }
    //disable timer interrupt!
    else 
        {asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(0));};
	timeout_event_head = timeout_event_head->next;
}

void timer_handler_begin(){
    unsigned long long cntpct_el0;
    asm volatile ("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    unsigned long long cntfrq_el0;
    asm volatile ("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));

	unsigned long long time = cntpct_el0/cntfrq_el0;
    uart_hex(time);
    //asm volatile ("msr cntp_tval_el0, %0"::"r"(2*cntfrq_el0));
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
