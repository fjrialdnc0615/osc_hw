#include "uart.h"
#include "mailbox.h"
#include "utils.h"
#include "shell.h"
#include "dtb.h"
#include "_cpio.h"
#include "except.h"
#include "timer.h"
#include "allocator.h"

struct timeout_event *timeout_event_head=NULL;
typedef void (*timer_callback)(char *);

void print_message(char *msg)
{
    uart_puts(msg);
}

void set_timeout(char *message, char *_time)
{
    unsigned int time = utils_str2uint_dec(_time);
    add_timer(print_message, message, time);
}

void add_timer(timer_callback cb, char *msg, unsigned long duration){

	unsigned long long cntpct_el0;
    asm volatile ("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    unsigned long long cntfrq_el0;
    asm volatile ("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));

	struct timeout_event* event = (struct timeout_event *)simple_malloc(sizeof(struct timeout_event));
	event->register_time = cntpct_el0/cntfrq_el0;
	event->callback = cb;
	
	//duration + current time
	event->exe_time = duration*cntfrq_el0 + cntpct_el0;

	for(int i=0;i<20;i++){
		event->msg[i] = msg[i];
		if(msg[i]=='\0') break;
	}
	
	struct timeout_event* cur_ptr = timeout_event_head; 
	struct timeout_event* prev_ptr = NULL;
	while(1){
		//if now timeout_event_head contains thing
		if(cur_ptr!=NULL){
			//if current event is not the first one to happen
			if(event->exe_time>cur_ptr->exe_time){
				prev_ptr = cur_ptr;
				cur_ptr = cur_ptr->next;
			}
			//if current event is the first one to happen
			else{
				event->next = cur_ptr;
				event->prev = cur_ptr->prev;
				cur_ptr->prev = event;
				prev_ptr->next = event;
				//event->next->prev = event;
				if(event->prev==NULL){
					timeout_event_head = event;
					prev_ptr = NULL;
				}
				break;
			}
		}
		//cur_ptr now point to null
		else{
			//cur_ptr doesnt contain a thing
			if(prev_ptr==NULL){
				cur_ptr = event;
				timeout_event_head = event;
				break;
			}
			//cur_ptr have previous link
			else{
				prev_ptr->next = event;
				event->prev = prev_ptr;
				break;
			}
		}
	}
	if(!prev_ptr){
    //setting current timeout event
	asm volatile ("msr cntp_cval_el0, %0"::"r"(timeout_event_head->exe_time));
	//enable timer
	asm volatile ("msr CNTP_CTL_EL0, %0"::"r"(1));
	}
}

void set_example_timeout(){
	set_timeout("first_timeout","5");
	set_timeout("second_timeout","7");
	set_timeout("third_timeout","9");
}
