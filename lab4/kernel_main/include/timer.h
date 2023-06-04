#define CORE0_TIMER_IRQ_CTRL ((volatile unsigned int *)(0x40000040))
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))

typedef void (*timer_callback)(char *);

typedef struct timeout_event{
  unsigned long register_time;
  unsigned long exe_time;
  timer_callback callback;
  char msg[20];
  struct timeout_event *prev, *next;
}timeout_event;


void set_timeout(char *message, char *_time);
void print_message(char *msg);
void add_timer(timer_callback cb, char *msg, unsigned long duration);
extern timeout_event *timeout_event_head;
void set_example_timeout();
