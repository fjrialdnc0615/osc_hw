#define qemu_cpio_address	0x8000000
#define rpi3b_cpio_address	0x20000000

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
