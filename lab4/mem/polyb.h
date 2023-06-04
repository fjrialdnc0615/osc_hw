#ifndef POLYB_H
#define POLYB_H

#define NULL 0
#define ISSET(reg, bit)         ((reg & (1 << bit)) != 0)
#define nop asm volatile("nop")

#define MIN(a, b) (a <= b) ? a : b
#define ENDL y_putchar('\r'); y_putchar('\n');

#define MRS(reg) \
    unsigned long long reg = 0; \
    asm volatile ("mrs %0, " #reg : "=r" (reg));

#define MSR(reg) \
    asm volatile ("msr " #reg ", %0" :: "r" (reg));

// ======================== SIZE TYPES ========================
typedef unsigned long long u64;
typedef unsigned u32;
typedef int i32;

// ===================== YOSIX FUNCTIONS ======================
// basic io
void y_putchar(char c);
char y_getc();

// async io
/*
 * interrupt driven async io,
 * require caller-side reentrant protection,
 * when reading:
 *  - if ready == buf_sz; request is fulfilled and removed
 */
struct y_aio_req {
    char *buf;
    unsigned buf_sz;
    unsigned ready;
    struct y_aio_req *next;
};
void y_aio_read(struct y_aio_req *req);
void y_aio_pop();
void y_process_aio();

// addresses
char *y_get_cpio_addr();
char *y_get_dtb_addr();

// mailbox
#define MBOX_GET_BOARD_REVISION 0x00010002
#define MBOX_GET_ARM_MEMORY     0x00010005

extern volatile unsigned int __attribute__((aligned(16))) y_mailbox[36];

void y_mailbox_clear(int c);
void y_mailbox_call();

// timeout
struct timeout {
    unsigned long long ctr;
    struct timeout *next;
    void (*cb)(void *userdata);
    void *userdata;
};
void y_set_timeout(unsigned seconds, void (*cb)(void *userdata), void *userdata);
void y_process_timeout();

// system
void y_init();

// ===================== STRING FUNCTIONS =====================
// x-str
void y_itoa(unsigned int i); // min_lib

// str-out
void y_puthex(unsigned int d);
void y_puts(const char *s);

// str-x
unsigned long long y_a16toi(const char *a, int n);

// str-misc
int y_strlen(const char *s);
int y_strncmp(const char *a, const char *b, int n);
const char *y_strnstr(const char *a, const char *b, int n);

// mem
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
void y_balloc_init(struct y_balloc *header);
void *y_balloc(u64 sz);
void y_bfree(void *ptr);
// ======================== YSH SHELL =========================
#define CMD_BUF_SIZE 1024
struct ysh_func_t
{
    const char *name;
    const char *help_str;
    int (*f)(int argc, const char **argv);
};
struct ysh
{
    struct ysh_func_t *funcs;
    int func_count;
};
struct ysh_context
{
    struct ysh *sh;
    char cmd[CMD_BUF_SIZE];
    struct y_aio_req req;
    int read_i;
};
struct ysh_context *ysh_async_begin(struct ysh *sh);
int ysh_async_poll(struct ysh_context *ctx);
int ysh_do_one(struct ysh_context *ctx);

// ========================== YLIBS ===========================
struct y_file_t
{
    void *meta; // store fs-related structs
    char *fn;
    char *data;
    unsigned long size;
};
struct y_files_t
{
    struct y_file_t *files;
    int count;
};
struct y_files_t cpio_read_files(char *ptr);

#define FDT_BEGIN_NODE  0x00000001
#define FDT_END_NODE    0x00000002
#define FDT_PROP        0x00000003
#define FDT_NOP         0x00000004
#define FDT_END         0x00000009
struct fdt_prop
{
    unsigned int len;
    unsigned int nameoff;
};
struct fdt_cb_ctx
{
    const char *strings;
    void *userdata;
    const char *cur;
    struct fdt_prop *prop;
    int depth;
};

typedef int (*dtb_cb)(unsigned int type, struct fdt_cb_ctx *ctx);

void dtb_traverse(char *ptr, dtb_cb cb, void *userdata);

// ========================== YUTILS ==========================
char *yu_align(char *addr, unsigned alignment);
char *yu_advance_cstr(char *cstr);
unsigned yu_to_little(unsigned big);

// ======================== INTERNALS =========================
/*
 * PRIORITY TABLE
 *
 */
struct irq_task {
    void (*cb)(void *userdata);
    void *userdata;
    int priority;
    struct irq_task *next;
};
void enqueue_irq_task(struct irq_task *task);

#endif
