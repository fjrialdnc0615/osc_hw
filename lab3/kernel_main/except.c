#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
#include "_cpio.h"
#include "except.h"
#include "timer.h"
#include "allocator.h"
#define AUX_MU_IO	   ((volatile unsigned int*)(0x3f215040))
#define AUX_MU_IER_REG ((volatile unsigned int*)(0x3f215044))

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }
void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

task *task_queue_head = 0, *task_queue_tail = 0;


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
    struct timeout_event* next = timeout_event_head->next;
    if(next!=NULL){
        asm volatile ("msr cntp_cval_el0, %0"::"r"(next->exe_time));
    }
    //disable timer interrupt!
    else 
        {asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(0));};
	timeout_event_head = timeout_event_head->next;
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


void add_task(task_callback cb, unsigned int priority)
{
    task *new_task = (task *)simple_malloc(sizeof(task));
    new_task->priority = priority;
    new_task->callback = cb;
    new_task->next = 0;
    new_task->prev = 0;
    if (task_queue_head == 0)
    {
        task_queue_head = new_task;
        task_queue_tail = new_task;
    }
    else
    {
        task *cur = task_queue_head;
        while (cur)
        {
			//if current task is the higher than next task
            if (new_task->priority > cur->priority)
                break;
            cur = cur->next;
        }
		//if current task is the lowest level
        if (cur == 0)
        { // cur at end
            new_task->prev = task_queue_tail;
            task_queue_tail->next = new_task;
            task_queue_tail = new_task;
        }
		//if current task is the highest level
        else if (cur->prev == 0)
        { // cur at head
            new_task->next = cur;
            (task_queue_head)->prev = new_task;
            task_queue_head = new_task;
        }
        else
        { // cur at middle
            new_task->next = cur;
            new_task->prev = cur->prev;
            (cur->prev)->next = new_task;
            cur->prev = new_task;
        }
    }
}

void exec_task()
{
    while (1)
    {
        task_queue_head->callback(task_queue_head->arg);
        disable_interrupt();
        task_queue_head = task_queue_head->next;
        if (task_queue_head)
        {
            task_queue_head->prev = 0;
        }
        else
        {
            task_queue_head = task_queue_tail = 0;
            return;
        }
        enable_interrupt();
    }
}


