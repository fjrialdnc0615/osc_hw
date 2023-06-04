#include "polyb.h"
#include <stdio.h>
struct nalloc_node
{	
	//linked list
    struct nalloc_node *next;
    u32 sz;
    u64 SANITY;
};

void nalloc_init(void *p, u64 sz, u32 usz)
{
	//p is a mem address and make it point to nalloc node
    struct nalloc_node *cur = (struct nalloc_node *) p;
    cur->next = 0;
    cur->sz = usz;
    cur->SANITY = 0xDEADBEEF;
	//自己本身nalloc_node的size+放入ptr的size(point到linklist的頭)
    u64 fsz = sizeof(struct nalloc_node) + usz;
	//每一次都加一次fsz(該node+裡面放的ptr)把每一塊nalloc都割好
    //因為只有一個page的限制，因此i不能大過sz(4096)
    for (u64 i = fsz; i < sz; i += fsz) {
        struct nalloc_node *h = (struct nalloc_node *)((u64)p + i);
		h->sz = usz;
        h->next = 0;
        h->SANITY = 0xDEADBEEF;
        //current nalloc -> next = new nalloc node, which is after current node
		cur->next = h;
		//current become next so as to go next one
        cur = h;
    }
}

void *nalloc(struct nalloc_node *head)
{
    struct nalloc_node *cur = head->next;
    if (cur) {
		//head會指向current要用的下一個，因為這一個要拿來用了
        head->next = cur->next;
        return (void *) (((u64) cur) + sizeof(struct nalloc_node));
    }
    return NULL;
}

void nfree(struct nalloc_node *head, void *p)
{
	//成立一個空的nalloc node並指向當前被用的address
    struct nalloc_node *n = (struct nalloc_node *)((u64)p - sizeof(struct nalloc_node));
    //讓當前的n的next指向原本head指向下一個空的記憶體位址= n->empty_nalloc_node
	n->next = head->next;
	//讓head指向當前的n= head->n->empty_nalloc_node
    head->next = n;
}

struct y_balloc *_global_allocator = 0;

/*
 * y_balloc reserves the first N bytes for The Array, and then allocates a few
 * more pages for itself for node allocation.
 */
void y_balloc_init(struct y_balloc *header)
{
	//header_size = struct y_balloc + array of u64 values
    u64 header_sz = sizeof(struct y_balloc);
    //10w/4096 ->entries_sz is used to reserve bitmap array(which contains 24 ptr size)
	u64 entries_sz = (header->sz / header->page_sz) * sizeof(y_balloc_entry);
	//freelist have n orders then it will contain n+1 order linked list and linked list is ptr so this is db ptr
    u64 freelist_sz = (header->order + 1) * sizeof(u64);
	//store address for entries ptr(header_position + header_sz)
    header->entries = (y_balloc_entry *) ((u64) header + header_sz);
    //store address for freelist ptr(entries_position + entries_sz)
	header->freelist = (struct y_balloc_node **) (((u64) header->entries) + entries_sz);
	//the location can be allocated for pages
    header->mem = (void *) (((u64) header->freelist) + freelist_sz);
    //10w - header_sz - entries_sz - freelist_sz
	header->sz -= header_sz + entries_sz + freelist_sz;
	//initializing
	for (int i = 0; i <= header->order; i++)
        header->freelist[i] = 0;

    // initialize a node-specialized allocator at the lowest order memory
    // everytime the nallocator is not able to provide, add 2x extra pages for
    // it. This couples the implementation of balloc and nalloc, as it should,
    // since all other use cases should utilize the generic malloc.
    // WE USE ONE PAGE HERE ALREADY
    
	//nalloc first place, nalloc is for allocating the loc for the first nalloc node
	struct nalloc_node *nhead = (struct nalloc_node *) header->mem;
	//y_balloc_entry is an typedef int which has CHILD stands for -999(they are the same),
	//but cpp will invoke error if (y_balloc_entry) and int
    header->entries[0] = (y_balloc_entry) -1;
    //init the nalloc to make it point to each node in i order freelist 8byte->64 
	nalloc_init((void *) nhead, header->page_sz, sizeof(struct y_balloc_node));
    header->nhead = nhead;
	
	//size=10w page_sz=4096
    u64 page_count = header->sz / header->page_sz;
    // MAYBE assert pc > 0 ?
	// order = 4 because (4<11)
    header->order = MIN(63 - __builtin_clzll(page_count), header->order);
	//current order start from 4 and if frame >= page_count then it breaks
	for (u64 frame = 1, cur_order = header->order; frame < page_count;) {
        u64 fcount = (1 << cur_order);

		//if 當前frame加上當前order會產生的frame大於總共需要的page就break
        if (frame + fcount > page_count) {
            cur_order--;
            continue;
        }
        //會變成(frame,order): (1,4) (17,2) (21,1) (23,0)
		//所以order4的index就會在1且下個order的index就在16個之後
		header->entries[frame] = (y_balloc_entry) cur_order;
		
		//建立一個freelist node 該node會知道現在指向bitmap的哪一個index
		struct y_balloc_node *n = (struct y_balloc_node *) nalloc(nhead);
        n->idx = frame;
        n->next = 0;

        // TODO: linked list
        //建立一個指向某一特定order之linklist的ptr
		struct y_balloc_node **cur = &header->freelist[cur_order];
        //對其中一個order head的後面做insert
		//假如當前head=null->就直接將當前的head指向新增的freenode
		while (*cur){
            cur = &(*cur)->next;
			}
		*cur = n;
		//假如是(1,4)那2~16(<17)都會是is_child
        for (u64 j = frame + 1; j < frame + fcount; j++)
            header->entries[j] = IS_CHILD;
		//因為都已經放好frame~frame+fcount-1的bitmap了，所以從frame+=fcount開始
        frame += fcount;
    }
	//_global_allocator指向header
    _global_allocator = header;
}
#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

void *y_balloc(u64 sz)
{
	//pc = org size / page_size + 1(if there is sth left)
    u64 pc = sz / _global_allocator->page_sz + ((sz % _global_allocator->page_sz) > 0);
    //if pc = 2 then it will be 63-62 = 1; it means we need to allocate first order
	u64 h = 63 - __builtin_clzll(pc);
	//if now is 11 then order1 cannot fulfill 3pages so you need to add 1 order->2
    u64 order = MIN(h + ((pc - (1 << h)) > 0), _global_allocator->order);
	//check if current order has available index, if not go to upper order
	i32 next_lowest_avail_order = -1;
	//next_lowest_avail_order will get the order that has index for allocate
    for (u32 c = order; c <= _global_allocator->order; c++)
        if (_global_allocator->freelist[c]) {
            next_lowest_avail_order = (i32) c;
            break;
        }
	//if no upper order has index then it cannot allocate so just return!
    if (next_lowest_avail_order < 0)
        return NULL;
#ifdef DEBUG
    printf("]] trying to allocate %llu bytes\n", sz);
    printf("]] %llu pages is required, order=%llu, currently lowest available is %d\n", pc, order, next_lowest_avail_order);
#endif
	
	//splitting phase 
    //if c > order表示當前的order比需要的order大所以要做切割直到切到當前需要的order為止
	for (u32 c = next_lowest_avail_order; c > order; c--) {
        struct y_balloc_node *n = _global_allocator->freelist[c];
		if (!n)
            break;
#ifdef DEBUG
        printf("]]] splitting an order %u block\n", c);
#endif
		//因為現在的n已經被用來切了，也算是被用掉了，所以指向下一個index，假如沒有就null
        _global_allocator->freelist[c] = n->next;
		//被切掉之後會分成兩個order-1的子節點n1 n2
		//n1 n2 會contain c-1的pages
        struct y_balloc_node *n1 = (struct y_balloc_node *) nalloc(_global_allocator->nhead);
		struct y_balloc_node *n2 = (struct y_balloc_node *) nalloc(_global_allocator->nhead);
		//所以n1會繼承當前n的位置，而n2會繼承n1+(n1包含的pages的位置)
		n1->idx = n->idx;
        n2->idx = n1->idx + (1 << (c - 1));
		//因為n1和n2是同order因此n1的next可以直接指向n2
        n1->next = n2;
		//而n2的next會指向當前order有的index，假如沒有就指向null
        n2->next = _global_allocator->freelist[c - 1];
		//因此freelist在指向當前切完order的freelist時會先指向n1再n2再原本的index
        _global_allocator->freelist[c-1] = n1;
		//entries[n1->idx]代表第幾個entry，並在此放入目前n1與n2的order
        _global_allocator->entries[n1->idx] = (y_balloc_entry) (c - 1);
        _global_allocator->entries[n2->idx] = (y_balloc_entry) (c - 1);
		//free掉當前node的nalloc node
        nfree(_global_allocator->nhead, n);
    }
    struct y_balloc_node *n = _global_allocator->freelist[order];
    if (!n)
        return NULL;
	
	//這邊終於開始使用當初需要的位置大小的order e.g. 12000->要用一個order2
    //因為使用了該order的一個位置因此目前的index要指向下一個空的index或是null
	_global_allocator->freelist[order] = n->next;
    //當前order所指向的index因為被佔用了，因此把裏面的數字*(-1)再-1
	_global_allocator->entries[n->idx] = (y_balloc_entry) (((i32) _global_allocator->entries[n->idx] * -1) - 1);
    //建立一個ret的指標，作為可用的記憶體位址，計算方式=存放page的位置+(idx*page_size)
	void *ret = (void *) ((u64)(_global_allocator->mem) + (u64)(n->idx * (_global_allocator->page_sz)));
    //free掉當前的n因為被用掉了
	nfree(_global_allocator->nhead, n);
    return ret;
}

void y_bfree(void *ptr)
{
    //抓到目前在第幾個page(ptr-mem = 當前page的address，再除上page size就可以得到當前page的位置idx)
    u32 idx = (((u64) ptr) - ((u64)_global_allocator->mem)) / _global_allocator->page_sz;
    //從目前的位置撈出原本的使用order大小(因為原本有乘上-1再減1所以要轉回去)
    i32 order = ((i32) _global_allocator->entries[idx] + 1) * -1;
#ifdef DEBUG
    printf("[[ freeing blocks of order %d\n", order);
    printf("[[ freeing blocks of idx %d\n\n", idx);
#endif
    //建立一個freelist node
    struct y_balloc_node *n = (struct y_balloc_node *) nalloc(_global_allocator->nhead);
    //讓目前的freelist node裡的member指向算出的idx
    n->idx = idx;
    //用於找到當前的idx的buddy e.g. 用到3、4個page就會是order2 假如目前是idx5 則buddy為1，若1為空直接merge
    u32 neighbour_idx =  n->idx ^ (1 << order);
    // check neighbour in entries
    // buddy的order若與當前的order相同(都為正(可以被free)且為同個order)，才會進入此while 
    while (_global_allocator->entries[neighbour_idx] == order) {
        // find it from freelist
        // 建立一個freelist node指向該order 以及prev_slot指向該order的freelist
        struct y_balloc_node *c = _global_allocator->freelist[order], **prev_slot = &_global_allocator->freelist[order];
        // 如果當前的freelist node指向buddy的index，若不是就往前一直走
        while (c) {
            if (c->idx == neighbour_idx) {
#ifdef DEBUG
                printf("]]] merging two order %d blocks\n", order);
#endif
                //當前freelist node指向buddy index進入該if並且建立一個free_list node ptr 
                struct y_balloc_node *merged = (struct y_balloc_node *) nalloc(_global_allocator->nhead);
                //要merge的index必須是比較小的這樣才不會覆蓋到其他的node
                merged->idx = MIN(n->idx,neighbour_idx);
                //因為merge掉之後會變成order+1的freelist node所以當前freelist的node就要被去掉
                //該order的freelist從指向當前的buddy指向buddy的下一個(不用擔心當前free的node會在裡面因為還沒進去)
                *prev_slot = c->next;
                //free掉這兩個node
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
        puts("!!!BUG!!!: FREE NEIGHBOUR NOT IN FREELIST");
#endif
        while(1);
next_order:
        order++;
        neighbour_idx =  n->idx ^ (1 << order);
    }
    //假如run完所有的merge(無法再merge的時候)，就進入將merge node(n)塞入freelist的階段
    //將當前的merge node塞入目前的freelist的head 並將原本的freelist的head作為當前merge node的下一個
    //在bitmap上specify當前的node是屬於哪一個node
    n->next = _global_allocator->freelist[order];
    _global_allocator->entries[n->idx] = (y_balloc_entry ) order;
    _global_allocator->freelist[order] = n;
}
