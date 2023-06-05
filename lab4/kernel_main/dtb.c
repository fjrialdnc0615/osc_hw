#include "poly.h"
#define UNUSED(X) (void)(X)

int space = 0;
char * cpio_addr;

uint32_t get_le2be_uint(const void *p)
{
    // transfer little endian to big endian
    const unsigned char *bytes = p; //e.g. 0x12345678
    uint32_t res = bytes[3]; //make the biggest byte to smallest number e.g.0x00000012
    res |= bytes[2] << 8;  //0x00003412
    res |= bytes[1] << 16; //0x00563412
    res |= bytes[0] << 24; //0x78563412
    return res;
}

void send_space(int n){
	//use it to do indent(when there is a subnode in a node then add more)
  while (n--) uart_puts(" ");
}

int parse_struct(fdt_callback cb, uintptr_t cur_ptr, uintptr_t strings_ptr, uint32_t totalsize){
	//set the end
    uintptr_t end_ptr = cur_ptr + totalsize;

    while (cur_ptr < end_ptr)
    {
		//from cur_ptr extracts current tag
        uint32_t token = get_le2be_uint((char *)cur_ptr);
		//because the information in tag is followed by cur_ptr(only)
        cur_ptr += 4;
        switch (token)
        {
			//if current node is FDT_BEGIN_NODE:
            case FDT_BEGIN_NODE:
                //cur_ptr store the name of node(followed by tag)
				cb(token, (char *)cur_ptr, NULL, 0);
				//after print the name, we need to pad to 4bytes
                cur_ptr += align_len(strlen((char *)cur_ptr), 4);   
                break;
            case FDT_END_NODE:
				//This token has no extra data; so it is followed immediately by the next token.
                cb(token, NULL, NULL, 0);
                break;
            case FDT_PROP:{
				/*
					This data consists first of the property’s length and name:
					struct {
						uint32_t len;
						uint32_t nameoff //it gives an offset into the strings block
						}
				*/
				//so len take len
                uint32_t len = get_le2be_uint((char *)cur_ptr);
                cur_ptr += 4;
				//nameoff take nameoff
                uint32_t nameoff = get_le2be_uint((char *)cur_ptr);
				//then the following cur_ptr is given as a byte string of length len.
				//so we will pass it.
                cur_ptr += 4;
				//strings_ptr + offset_value can get the target in string block
                cb(token, (char *)(strings_ptr + nameoff), (void *)cur_ptr, len);
                cur_ptr +=  align_len(len, 4);;
                break;
            }
            case FDT_NOP:
                //nothing in this tag, so we can just pass it.
				cb(token, NULL, NULL, 0);
                break;
            case FDT_END:
                //nothing in this tag, so we can just pass it.
                cb(token, NULL, NULL, 0);
                return 0;
            default:;
                return -1;
        }
    }
	return -1;
}



void print_dtb(int type, const char *name, const void *data, uint32_t size)
{
	UNUSED(data);
	UNUSED(size);
    switch (type)
    {
    case FDT_BEGIN_NODE:
        uart_send('\n');
		//if this is subnode then it will be indent
        send_space(space);
		//output the name of the node
        uart_puts((char *)name);
		//then it will be followed by other things so add {
        uart_puts("{\n ");
        //the node after this node is subnode so we need space to do indent
		space++;
        break;

    case FDT_END_NODE:
		//the end of node(subnode) so we use another line to add }
        uart_puts("\n");
		//this node comes to an end so no indent in this node needed.
        space--;
		//if the end is an subnode it still need to be indent by father nodes
        if (space>0) send_space(space);
        uart_puts("}\n");
        break;

    case FDT_NOP:
        break;

    case FDT_PROP:
		//each content should be blocked by space or it will become hard to divide...
        send_space(space);
		//output the content of the node
        uart_puts((char*)name);
        break;

    case FDT_END:
        break;
    }
}

//using this function to capture the address of cpio from the device start(in mainc)
void get_cpio_addr(int type, const char *name, const void *data, uint32_t size)
{
	UNUSED(size);
	//the property of byte string of length len in qemu is 0x80000000
    if(type==FDT_PROP&&str_compare((char*)name,"linux,initrd-start")){
        cpio_addr=(char *)(uintptr_t)get_le2be_uint(data);
        uart_puts("cpio_addr is: ");
        uart_hex((uintptr_t)get_le2be_uint(data));
        uart_send('\n');
    }
}


int fdt_traverse(fdt_callback cb, void* _dtb){
	uintptr_t _dtb_ptr = (uintptr_t)_dtb;
	uart_puts("dtb address is :\n");
	uart_hex(_dtb_ptr);
	uart_send('\n');

	//from ptr address extract header
	fdt_header *header = (fdt_header *)_dtb_ptr;
	if(get_le2be_uint(&(header->magic))!=0xd00dfeed){
		uart_puts("the header magic is wrong!!\n");
		return -1;
	}
	
	//get the totalsize of devicetree.
	uint32_t totalsize = get_le2be_uint(&(header->totalsize));
	//get the address of struct_ptr by get the offset value from off_dt_struct then add to dtb_ptr
	uintptr_t struct_ptr = _dtb_ptr+get_le2be_uint(&(header->off_dt_struct));
	//get the address of strings_ptr by get the offset value from off_dt_struct then add to dtb_ptr
	uintptr_t strings_ptr = _dtb_ptr+get_le2be_uint(&(header->off_dt_strings));
	parse_struct(cb, struct_ptr, strings_ptr, totalsize);
	return 1;
}
