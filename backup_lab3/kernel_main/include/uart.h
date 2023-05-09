//timer interrupt checker and miniuart interrupt checker
extern volatile unsigned int *CORE0_INTERRUPT_SOURCE;
extern volatile unsigned int *AUX_MU_IIR_REG;
extern char input_buffer[128];

extern unsigned int buffer_arrow; 
extern unsigned int transmit_arrow; 
void uart_init();
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char *s);
void uart_hex(unsigned int n);
int uart_atoi(char* hex_pointer,int size);
void read_command(char* array_space);
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();
void enable_uart_irt();
