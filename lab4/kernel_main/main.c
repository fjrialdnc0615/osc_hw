#include "poly.h"

int doing_task = 0;
extern void *_dtb_ptr; //from start.s get _dtb_ptr
void main()
{
	// set up serial console
    uart_init();

	//allocator location
    void *mem = (void *)0x10000000;
    //locate at 0x10000000
    struct y_balloc *bhead = mem;
    bhead->order = 11;
    bhead->page_sz = 4096;
    bhead->sz = 0x3C000000;

	fdt_traverse(get_cpio_addr,_dtb_ptr);

    y_balloc_preinit(bhead);

    y_balloc_reserve(0, (void *) 0x1000);
    y_balloc_reserve((void *) 0x1000, (void *) 0x80000);
    y_balloc_reserve((void *) 0x80000, (void *) 0x80000 + 30000);
    y_balloc_reserve(cpio_addr, cpio_addr + 30000);
    y_balloc_reserve((char*) _dtb_ptr, (char*) _dtb_ptr+ 60000);
   
    y_balloc_init();
    uart_puts("Im here!!");
    y_malloc_init();

    balloc_state(bhead);


	unsigned long el = 0;
	asm volatile ("mrs %0, CurrentEL":"=r" (el)); //=r takes a register to store i/o
	uart_puts("Current exception level:");
	uart_hex(el>>2); //CurrentEL store EL level at [3:2]

	asm volatile ("mov %0, sp":"=r" (el));
	uart_puts("Current stack pointer address:");	uart_hex(el);
	uart_send('\n');
	read_file();
    
    
	enable_uart_irt();
	
	set_example_timeout();
	uart_send('#');

	while(1) asm volatile("wfe");

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
    
    while(1);
}


void irq_exception_handler_c(){
	//check if interrupt is issued by timer
	if(*CORE0_INTERRUPT_SOURCE & 2){
		add_task(timer_handler,0);
	}
	else if(*AUX_MU_IIR_REG & 4){
		add_task(receiver_handler,3);
		}
	else if(*AUX_MU_IIR_REG & 2)
		add_task(transmit_handler,2);

	if (!doing_task)
    {
        doing_task = 1;
        exec_task();
        doing_task = 0;
    }
}
void balloc_state(struct y_balloc *b)
{
    void *_p_args[2] = {0, 0};
    u64 sz = 0;
    for (i32 i = 0; i <= b->order; i++) {
        struct y_balloc_node *cur = b->freelist[i];
        u32 cnt = 0;
        while (cur) {
            cnt++;
            cur = cur->next;
        }
        sz += (1 << i) * cnt;
		uart_puts("ORDER ");
		uart_putu(i);
		uart_puts(" has ");
		uart_putu(cnt);
		uart_puts(" blocks available.\r\n");
		/*
        _p_args[0] = (void *) i; _p_args[1] = (void *) cnt;
        y_printf("ORDER %u has %u blocks available.\r\n", _p_args);
		*/
    }
    uart_puts("AVAILABLE PAGES: ");
    uart_putu(sz);
    ENDL;
}