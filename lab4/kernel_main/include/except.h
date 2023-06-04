typedef void (*task_callback)(void *);

typedef struct task
{
    unsigned long priority;
    unsigned long duration;
    void *arg;
    task_callback callback;
    struct task *prev, *next;
} task;

void add_task(task_callback cb,unsigned int priority);
void exec_task();

void timer_handler();
void timer_handler_begin();
void receiver_handler();
void transmit_handler();

void enable_interrupt();
void disable_interrupt();
