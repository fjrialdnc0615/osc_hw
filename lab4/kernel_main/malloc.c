#include "poly.h"
#define PAGE_SIZE 0x1000 // 4KB

struct nalloc_node
{
    struct nalloc_node *next;
    u32 sz;
    u64 SANITY;
};

void nalloc_init(void *p, u64 sz, u32 usz)
{
    struct nalloc_node *cur = (struct nalloc_node *) p;
    cur->next = 0;
    cur->sz = usz;
    cur->SANITY = 0xDEADBEEF;
    u64 fsz = sizeof(struct nalloc_node) + usz;
    for (u64 i = fsz; i < sz; i += fsz) {
        struct nalloc_node *h = (struct nalloc_node *)(p + i);
        h->sz = usz;
        h->next = 0;
        h->SANITY = 0xDEADBEEF;
        cur->next = h;
        cur = h;
    }
}

void *nalloc(struct nalloc_node *head)
{
    struct nalloc_node *cur = head->next;
    if (cur) {
        head->next = cur->next;
        //current start address of node + header_sz so it will be storage addr 
        return (void *) (((u64) cur) + sizeof(struct nalloc_node));
    }
    //else rejuv_nalloc(head->sz);
}

void nfree(struct nalloc_node *head, void *p)
{
    struct nalloc_node *n = (struct nalloc_node *)(p - sizeof(struct nalloc_node));
    n->next = head->next;
    head->next = n;
}

struct y_balloc *_global_allocator = 0;
void *(*balloc_nalloc)(u64);
void *internal_nalloc(u64 dontcare)
{
    return nalloc(_global_allocator->nhead);
}

void y_balloc_reserve(void *start, void *end)
{
    uart_puts("\n\nthis is your start address");
    uart_puthexll((u64)start);ENDL;
    uart_puts("this is your end address");
    uart_puthexll((u64)end);ENDL;
    uart_puts("\r\nRESERVED\r\n");
    //TODO: calculate page sz exponent
    //i will start at beginning page and it will end 
    for (u64 i = (u64)start >> 12;
         i < ((u64) align_up(end, (1<<12))) >> 12;
         i++) {
        //uart_puts("this is page:");
        //uart_putu(i);ENDL;
        _global_allocator->entries[i] = -1;
    }
}

/*
 * header must point to a safe memory region that is big enough for entire entries map,
 * returns the end of the header, which should later be reserved: (header, preinit(header).
 */
void y_balloc_preinit(struct y_balloc *header)
{
    u64 page_count = header->sz / header->page_sz;
    u64 header_sz = sizeof(struct y_balloc);
    //so big y_balloc_entry is similar to int
    u64 entries_sz = page_count * sizeof(y_balloc_entry);
    uart_puts("Entries here:\n");
    uart_putu(entries_sz);
    u64 freelist_sz = (header->order + 1) * sizeof(u64);
    header->entries = (y_balloc_entry *) ((u64) header + header_sz);
    header->freelist = (struct y_balloc_node **) (((u64) header->entries) + entries_sz);
    header->mem = 0;
    _global_allocator = header;
    for (int i = 0; i <= header->order; i++)
        header->freelist[i] = 0;
    for (u64 i = 0; i < page_count; i++)
        header->entries[i] = 0;
    y_balloc_reserve(header, header + header_sz + entries_sz + freelist_sz);
}

/*
 * y_balloc reserves the first N bytes for The Array, and then allocates a few
 * more pages for itself for node allocation.
 */
void y_balloc_init()
{
    struct y_balloc *h = _global_allocator;
    u64 page_count = h->sz / h->page_sz, frame = 0;
    h->order = MIN(63 - __builtin_clzll(page_count), h->order);

    // initialize a node-specialized allocator at the first page available
    // the nallacator will be hooked up with generic malloc after it is available
    struct nalloc_node *nhead;
    {
        //frame will go to available page
        for (; h->entries[frame] < 0; frame++);
        nhead = (struct nalloc_node *) (frame << 12);
        nalloc_init((void *) nhead, 1 << 12, sizeof(struct y_balloc_node));
        balloc_nalloc = internal_nalloc;
        h->entries[frame++] = -1;
        h->nhead = nhead;
    }
    for (; frame < page_count;) {
        // find next available page
        uart_puts("\n\nframe occupy");
        uart_putu(frame);ENDL;

        for (; h->entries[frame] < 0; frame++);

        uart_puts("\n\nthis is start of avai frame: ");
        uart_putu(frame);ENDL;ENDL;
 

        // find next unavailable page
        u64 e = frame + 1;
        for (; h->entries[e] >= 0; e++){
          if(e==page_count)break;
        }
        uart_puts("\n\nthis is end of avai frame: ");
        uart_putu(e);ENDL;ENDL;
 

        for (u64 cur_order = h->order; frame < e;) {
            u64 fcount = (1 << cur_order);
            if (frame + fcount > e) {
                cur_order--;
                continue;
            }
            h->entries[frame] = (y_balloc_entry) cur_order;

            // TODO: linked list
            struct y_balloc_node **cur = &h->freelist[cur_order];
            while (*cur)
                cur = &(*cur)->next;
            struct y_balloc_node *n = balloc_nalloc(sizeof(struct y_balloc_node));
            n->idx = frame;
            n->next = 0;
            *cur = n;

            for (u64 j = frame + 1; j < frame + fcount; j++)
                h->entries[j] = IS_CHILD;
            frame += fcount;
        }
        uart_puts("I'm free!!!"); 
    }
}

void *y_balloc(u64 sz)
{
    u64 pc = sz / _global_allocator->page_sz + ((sz % _global_allocator->page_sz) > 0);
    u64 h = 63 - __builtin_clzll(pc);
    u64 order = MIN(h + ((pc - (1 << h)) > 0), _global_allocator->order);
    i32 next_lowest_avail_order = -1;
    for (u32 c = order; c <= _global_allocator->order; c++)
        if (_global_allocator->freelist[c]) {
            next_lowest_avail_order = (i32) c;
            break;
        }
    if (next_lowest_avail_order < 0)
        return NULL;
    for (u32 c = next_lowest_avail_order; c > order; c--) {
        struct y_balloc_node *n = _global_allocator->freelist[c];
        if (!n)
            break;
        _global_allocator->freelist[c] = n->next;
        struct y_balloc_node *n1 = balloc_nalloc(sizeof(struct y_balloc_node));
        struct y_balloc_node *n2 = balloc_nalloc(sizeof(struct y_balloc_node));
        n1->idx = n->idx;
        n2->idx = n1->idx + (1 << (c - 1));
        n1->next = n2;
        n2->next = _global_allocator->freelist[c - 1];
        _global_allocator->freelist[c - 1] = n1;
        _global_allocator->entries[n1->idx] = (y_balloc_entry) (c - 1);
        _global_allocator->entries[n2->idx] = (y_balloc_entry) (c - 1);
        nfree(_global_allocator->nhead, n);
    }
    struct y_balloc_node *n = _global_allocator->freelist[order];
    if (!n)
        return NULL;
    _global_allocator->freelist[order] = n->next;
    _global_allocator->entries[n->idx] = (y_balloc_entry) (((i32) _global_allocator->entries[n->idx] * -1) - 1);
    void *ret = (void *) (n->idx * (_global_allocator->page_sz));
    nfree(_global_allocator->nhead, n);
#ifdef DEBUG
    uart_puts("]] allocated"); ENDL;
#endif
    return ret;
}

void y_bfree(void *ptr)
{
    u32 idx = ((u64) ptr) / _global_allocator->page_sz;
    i32 order = ((i32) _global_allocator->entries[idx] + 1) * -1;
#ifdef DEBUG
    uart_puts("[[ freeing blocks of order ");
    uart_putu(order);
    ENDL;
#endif
    struct y_balloc_node *n = balloc_nalloc(sizeof(struct y_balloc_node));
    n->idx = idx;
    u32 neighbour_idx =  n->idx ^ (1 << order);
    // check neighbour in entries
    while (_global_allocator->entries[neighbour_idx] == order) {
        // find it from freelist
        struct y_balloc_node *c = _global_allocator->freelist[order], **prev_slot = &_global_allocator->freelist[order];
        while (c) {
            if (c->idx == neighbour_idx) {
                struct y_balloc_node *merged = balloc_nalloc(sizeof(struct y_balloc_node));
                _global_allocator->entries[neighbour_idx] = IS_CHILD;
                merged->idx = MIN(n->idx, neighbour_idx);
                *prev_slot = c->next;
                nfree(_global_allocator->nhead, n);
                nfree(_global_allocator->nhead, c);
                n = merged;
                goto next_order;
            }
            prev_slot = &c->next;
            c = c->next;
        }
        // it's not in freelist, BUG?!
#ifdef DEBUG
        uart_puts("!!!BUG!!!: FREE NEIGHBOUR NOT IN FREELIST\r\n");
#endif
        while(1);
next_order:
        order++;
        neighbour_idx =  n->idx ^ (1 << order);
    }
    n->next = _global_allocator->freelist[order];
    _global_allocator->entries[n->idx] = (y_balloc_entry ) order;
    _global_allocator->freelist[order] = n;
}

//binsize order be like: first for balloc, sec for 64 then 128, 256
u32 binsize[4] = {sizeof(struct y_balloc_node), 64, 128, 256};
struct nalloc_node *bins[4];
int rejuv_nalloc(int order)
{
    // assumption: bins[order]->next = NULL; bins[order] != NULL;
    //because current page is full occupied, so next of nalloc node will be in new page
    //new page will be allocated by balloc and init it with size captured by bins[order]
    struct nalloc_node *next = y_balloc(4096);
    if (!next)
        return -1;
    nalloc_init(next, 4096, bins[order]->sz);
    bins[order]->next = next;
    uart_puts("you are here for rejuv_nalloc\n");
    return 0;
}

void *y_malloc(u64 x)
{
    //i will be in [balloc_node, 64, 96, 128]
    for (int i = 0; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        //if current size of malloc is bigger than required size then use that malloc
        if (binsize[i] > x) {
            void *ret = nalloc(bins[i]);
            if (ret)
                return ret;
            // rejuvenate bins[i]
            if (rejuv_nalloc(i) < 0) {
                uart_puts("!!!BUG!!!: CANT REJUV BINS\r\n");
                while(1);
            }
            ret = nalloc(bins[i]);
            if (!ret) {
                uart_puts("!!!BUG!!!: CANT REJUV BINS\r\n");
                while(1);
            }
            return ret;
        }
    }
    // small allocator can't provide, propagate to balloc
    return y_balloc(x);
}

void y_malloc_init()
{
    // merge balloc_nallocator together with bins[0]
    bins[0] = _global_allocator->nhead;
    for (int i = 1; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        void *m = y_balloc(4096);
        // m is location, 4096 is page_sz and binsize[i] is size for that allocator
        nalloc_init(m, 4096, binsize[i]);
        bins[i] = m;
    }
    //after finish malloc_init balloc_nalloc become y_malloc
    balloc_nalloc = y_malloc;
}

void y_free(void *ptr)
{
    if (__builtin_ctzll((u64) ptr) >= 12) {
        uart_puts("BFREE");ENDL;
        y_bfree(ptr);
        return;
    }
    struct nalloc_node *header = ptr - sizeof(struct nalloc_node);
    for (int i = 0; i < sizeof(binsize)/sizeof(binsize[0]); i++) {
        if (binsize[i] == header->sz) {
            nfree(bins[i], ptr);
            return;
        }
    }
    uart_puts("!!BUG!!\r\n");
    ENDL;

}
