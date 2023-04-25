#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
extern void *_dtb_ptr; //from start.s get _dtb_ptr

void main()
{
	// set up serial console
    uart_init();
    // say hello
    uart_puts("Hello World!\n");
	fdt_traverse(get_cpio_addr,_dtb_ptr);
	shell();
}
