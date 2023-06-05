#include <stdint.h>
#include <stddef.h>

#ifndef POLY_H
#define POLY_H

//macro
typedef unsigned long long u64;
typedef unsigned u32;
typedef int i32;
#define ENDL uart_send('\r'); uart_send('\n');
#define MIN(a, b) (a <= b) ? a : b


//_cpio.h
#define qemu_cpio_address	0x8000000
#define rpi3b_cpio_address	0x20000000

extern char* cpio_addr;

struct file{
	struct cpio_header* file_header;
	char* file_name;
	char* file_content;
	int name_size;
	int file_size;
};

typedef struct cpio_header 
{
    // uses 8-byte	hexadecimal fields for all numbers
    char c_magic[6];    //determine whether this archive is written with little-endian or big-endian integers.
    char c_ino[8];      //determine when two entries refer to the same file.
    char c_mode[8];     //specifies	both the regular permissions and the file type.
    char c_uid[8];      // numeric user id
    char c_gid[8];      // numeric group id
    char c_nlink[8];    // number of links to this file.
    char c_mtime[8];    // Modification time of the file
    char c_filesize[8]; // size of the file
    char c_devmajor[8];
    char c_devminor[8];
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8]; // number of bytes in the pathname
    char c_check[8];    // always set to zero by writers and ignored by	readers.
}cpio_header;
void read_file();
void ls_command();
void cat_command(char* args1);

//allocator.h
void* simple_malloc(size_t size);

//dtb.h

/*
    structure block: located at a 4-byte aligned offset from the beginning of the devicetree blob
    token is a big-endian 32-bit integer, alligned on 32bit(padding 0)
*/

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009

typedef struct fdt_header
{
    uint32_t magic;             // contain the value 0xd00dfeed (big-endian).
    uint32_t totalsize;         // in byte
    uint32_t off_dt_struct;     // the offset in bytes of the structure block from the beginning of the header
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;   // the length in bytes of the strings block section
    uint32_t size_dt_struct;
} fdt_header;

typedef void (*fdt_callback)(int type, const char *name, const void *data, uint32_t size);
int fdt_traverse(fdt_callback cb, void* _dtb);
void send_space(int n);
int parse_struct(fdt_callback cb, uintptr_t cur_ptr, uintptr_t strings_ptr, uint32_t totalsize);
void print_dtb(int type, const char *name, const void *data, uint32_t size);
void get_cpio_addr(int type, const char *name, const void *data, uint32_t size);

//except.h
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

//exec.h
void exec_command(char* args1);

//gpio.h
#define MMIO_BASE       0x3F000000

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

//mailbox.h
int mbox_call(unsigned char ch);
void get_board_revision();
void get_size_and_memory_address();

//shell.h
void shell_command(char* input_string);

//timer.h
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

//uart.h
//timer interrupt checker and miniuart interrupt checker
extern char input_buffer[128];
#define AUX_MU_IIR_REG ((unsigned int *) (0x3f215048))
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
void uart_puthex(unsigned int d);
void uart_puthexll(u64 d);
void uart_putu(u64 x);

//utils.h
int str_compare(char* a, char* b);
int align(int size_of_content);
size_t strlen(const char *s);
uint32_t align_len(uint32_t size, int alignment);
unsigned int utils_str2uint_dec(const char *str);
char *align_up(char *addr, unsigned alignment);

//malloc.h
void *y_malloc(u64 sz);
void y_free(void *ptr);
void y_malloc_init();
char *y_alloc(unsigned sz);
struct nalloc_node;
struct y_balloc_node {
    u32 idx;
    struct y_balloc_node *next;
};
typedef enum
{
    IS_CHILD = -9999,
} y_balloc_val;

typedef y_balloc_val y_balloc_entry;
struct y_balloc
{
    unsigned order;
    unsigned long long sz;
    unsigned page_sz;
    struct nalloc_node *nhead;
    y_balloc_entry *entries;
    struct y_balloc_node **freelist;
    void *mem;
    /* it is followed by these:
     * [ y_balloc_entry ] \
     * [ y_balloc_entry ] --> * (sz/page_sz)
     * [ y_balloc_entry ] /
     * [ struct y_balloc_node *freelist_order_0 ] (u64)
     * [ struct y_balloc_node *freelist_order_1 ] (u64)
     * [ ... ]
     * [ struct y_balloc_node *freelist_order_i ] (u64)
     */
};
// populate the freelist and entries of a balloc region
void y_balloc_preinit(struct y_balloc *header);
void y_balloc_reserve(void *start, void *end);
void y_balloc_init();
void *y_balloc(u64 sz);
void y_bfree(void *ptr);



#endif // POLY_H