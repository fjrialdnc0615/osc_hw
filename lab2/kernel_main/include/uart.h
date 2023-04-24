
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
