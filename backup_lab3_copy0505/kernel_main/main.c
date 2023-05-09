#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
#include "_cpio.h"
#include "except.h"

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))
#define AUX_MU_IIR_REG ((volatile unsigned int*)(0x3f215048))
unsigned int timer_begin = 1;
extern void *_dtb_ptr; //from start.s get _dtb_ptr
void main()
{
	// set up serial console
    uart_init();

	unsigned long el = 0;
	asm volatile ("mrs %0, CurrentEL":"=r" (el)); //=r takes a register to store i/o
	uart_puts("Current exception level:");
	uart_hex(el>>2); //CurrentEL store EL level at [3:2]

	asm volatile ("mov %0, sp":"=r" (el));
	uart_puts("Current stack pointer address:");	uart_hex(el);
	uart_send('\n');
	fdt_traverse(get_cpio_addr,_dtb_ptr);
	read_file();
	enable_uart_irt();
	uart_send('#');
	while(1) asm volatile("nop");

	asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(0));
}
void exception_handler_c(){

	uart_puts("In excpetion!!\n");

	//read spsr_el1
	unsigned long long spsr_el1 = 0;
	asm volatile ("mrs %0, spsr_el1":"=r"(spsr_el1));
	uart_puts("spsr_el1: ");
	uart_hex(spsr_el1);
	uart_send('\n');
	
	//read elr_el1
	unsigned long long elr_el1 = 0;
	asm volatile ("mrs %0, elr_el1":"=r"(elr_el1));
	uart_puts("elr_el1: ");
	uart_hex(elr_el1);
	uart_send('\n');

	
	//read esr_el1
	unsigned long long esr_el1 = 0;
	asm volatile ("mrs %0, esr_el1":"=r"(esr_el1));
	uart_puts("esr_el1: ");
	uart_hex(esr_el1);
	uart_send('\n');

	//print ec
	unsigned ec = (esr_el1 >> 26) & 63;
	uart_puts("ec: ");
	uart_hex(ec);
	uart_send('\n');

}
void irq_exception_handler_c(){
	//check if interrupt is issued by timer
	if((*CORE0_INTERRUPT_SOURCE & 2) && (timer_begin==1)){
		timer_handler_begin();
	}
	else if((*CORE0_INTERRUPT_SOURCE & 2) && (timer_begin==0)){
		timer_handler();
	}
	else if(*AUX_MU_IIR_REG & 4)
		receiver_handler();
	else if(*AUX_MU_IIR_REG & 2)
		transmit_handler();
}


