#include "poly.h"

char cpio_array_space[128] = {0}; //using for cat
#define MAX_FILES 128

struct file *files = NULL;
int max_files = 0;
int file_num = 0;

void read_file() {
    char *addr = (char *) cpio_addr;
    int header_size = sizeof(struct cpio_header);

    while (1) {
        if (file_num >= max_files) {
            max_files += 10;
            files = (struct file *) y_malloc(max_files * sizeof(struct file));
        }

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
}

void ls_command() {
    for (int i = 0; i < file_num; i++) {
        uart_puts(files[i].file_name);
        uart_send('\n');
    }
}

void cat_command(char* args1) {
    //uart_puts("you are in cat:\n ");
    //read_command(cpio_array_space);
    //char *input_string = cpio_array_space;
    int check_found = 0;
    for (int i = 0; i < file_num; i++) {
        if (str_compare(args1, files[i].file_name)) {
            for (int j = 0; j < files[i].file_size; j++)
                uart_send(files[i].file_content[j]);
            check_found = 1;
            break;
        }
    }
    if (check_found == 0)
        uart_puts("file not found!!!\n");
}

