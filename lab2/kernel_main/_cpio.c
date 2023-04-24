#include "uart.h"
#include "_cpio.h"
#include "utils.h"

struct file{
	struct cpio_header* file_header;
	char* file_name;
	char* file_content;
	int name_size;
	int file_size;
};



#define MAX_FILES 128

struct file files[MAX_FILES];

void read_file(int check) {
    char *addr = (char *) qemu_cpio_address;
    int header_size = sizeof(struct cpio_header);

    int file_num = 0;

    while (1) {
        struct cpio_header *cpio_addr = (struct cpio_header *) addr;

        files[file_num].file_header = cpio_addr;
        files[file_num].name_size = uart_atoi(cpio_addr->c_namesize, (int) sizeof(cpio_addr->c_namesize));
        files[file_num].file_size = uart_atoi(cpio_addr->c_filesize, (int) sizeof(cpio_addr->c_filesize));

        addr += header_size; // address from header to file name

        if (str_compare(addr, "TRAILER!!!"))
            break;
        files[file_num].file_name = addr;
        // address from file name to content
        addr += files[file_num].name_size + align(header_size + files[file_num].name_size);

        if (files[file_num].file_size != 0) {
            files[file_num].file_content = addr;

            // address from file content to another header
            addr += files[file_num].file_size + align(files[file_num].file_size);
        } else
            files[file_num].file_content = "";
        file_num++;
    }

    if (check == 1) {
        for (int i = 0; i < file_num; i++) {
            uart_puts(files[i].file_name);
            uart_send('\n');
        }
    } else {
        uart_puts("you are in cat:\n ");
        char array_space[128] = {0};
        read_command(array_space);
        char *input_string = array_space;

        for (int i = 0; i < file_num; i++) {
            if (str_compare(input_string, files[i].file_name))
                uart_puts(files[i].file_content);
        }
    }
}


void ls_command()
{
	read_file(1);
}
void cat_command()
{
	read_file(0);
}
	
