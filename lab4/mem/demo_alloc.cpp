#include <iostream>
#include <vector>

#include "polyb.h"

struct nalloc_node
{
    struct nalloc_node *next;
    u32 sz;
    u64 SANITY;
};

void nalloc_init(void *p, u64 sz, u32 usz);
void *nalloc(struct nalloc_node *head);
void nfree(struct nalloc_node *head, void *p);

//to see available pages
void balloc_state(struct y_balloc *b)
{
    u64 sz = 0;
    for (i32 i = 0; i <= b->order; i++) {
		struct y_balloc_node *cur = b->freelist[i];
        u32 cnt = 0;
        //cur is freelist ptr, one cur point to one order of freelist
		while (cur) {
            cnt++;
            cur = cur->next;
        }
		//if now order = 4 then it contains 1<<4=16 pages
        sz += (1 << i) * cnt;
        std::cout << "ORDER " << i << " has " << cnt << " blocks available." << std::endl;
    }
    std::cout << "AVAILBLE PAGES: " << sz << std::endl;
}

int main()
{
    u64 sz = 100000;
	//request a real memory address
    void *mem = malloc(sz);
	//type casting mem to y_balloc(struct)
	struct y_balloc *bhead = (struct y_balloc *)mem;
    //the largest order will be 11
	bhead->order = 11;
    //each page contains 4096 bit(4kb)
	bhead->page_sz = 4096;
	//put size into obj bhead
    bhead->sz = sz;
	//init buddy_system requirements
    y_balloc_init(bhead);
    balloc_state(bhead);

	//allocate 12000 bits
    y_balloc(12000);
    std::cout << "ALLOCATED 12000" << std::endl;
    balloc_state(bhead);
    y_balloc(8000);
    std::cout << "ALLOCATED 8000" << std::endl;
    balloc_state(bhead);
    void *pp = y_balloc(16000);
    printf("pp = %llu \n",((unsigned long long)pp));
    std::cout << "ALLOCATED 16000" << std::endl;
    balloc_state(bhead);

    y_bfree(pp);
    std::cout << "FREED 16000" << std::endl;
    balloc_state(bhead);
    /*
    u64 sz = 5000, usz = 100;
    void *bp = malloc(sz);
    nalloc_init(bp, sz, usz);

    std::vector<void *> ptrs;
    void *p;
    while ((p = nalloc((struct nalloc_node *) bp)))
        ptrs.push_back(p);

    u64 c = ptrs.size();
    std::cout << "TOTAL SIZE: "  << sz << std::endl;
    std::cout << "ALLOCATED: " << c << std::endl;

    for (int i = 0; i < c; i++) {
        nfree((struct nalloc_node *) bp, ptrs.back());
        ptrs.pop_back();
    }
    std::cout << "ALLOCATED: " << ptrs.size() << std::endl;

    while ((p = nalloc((struct nalloc_node *) bp)))
        ptrs.push_back(p);
    std::cout << "ALLOCATED: " << ptrs.size() << std::endl;

     */
}
