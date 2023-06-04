#include <stdint.h>
#include <stddef.h>
int str_compare(const char* a, const char* b){
        int sentinel_value = 0;
        while(*a != '\0' || *b != '\0')
        {
                if(*a=='\n'){
                        a++;
                        continue;
                }
                if(*a==*b) sentinel_value = 1;
                else return 0;
                a++;
                b++;
        }
        return sentinel_value;
}
int align(int size_of_content){
	return size_of_content%4!=0? 4-size_of_content%4 : 0;
}
size_t strlen(const char *s) {
	size_t i = 0;
	while (s[i]) i++;
	return i+1;
}
uint32_t align_len(uint32_t size, int alignment) {
	return (size + alignment - 1) & -alignment;//using second complenent to do align
	/*
		e.g. 23 + 8 - 1 &(-8)
		to convert -8 to binary in 32-bit two's complement representation:
		8 in binary: 0000 0000 0000 0000 0000 0000 0000 1000
		Invert all the bits: 1111 1111 1111 1111 1111 1111 1111 0111
		Add 1: 1111 1111 1111 1111 1111 1111 1111 1000
		align size = 24;
	*/
}

unsigned int utils_str2uint_dec(const char *str)
{
    unsigned int value = 0u;

    while (*str)
    {
        value = value * 10u + (*str - '0');
        ++str;
    }
    return value;
}
