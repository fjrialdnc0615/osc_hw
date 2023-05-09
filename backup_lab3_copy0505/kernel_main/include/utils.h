#include <stdint.h>
#include <stddef.h>

int str_compare(char* a, char* b);
int align(int size_of_content);
size_t strlen(const char *s);
uint32_t align_len(uint32_t size, int alignment);
unsigned int utils_str2uint_dec(const char *str);
