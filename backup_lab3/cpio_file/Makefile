all: user.img
user.o: user.S
	aarch64-unknown-linux-gnu-gcc -c user.S -o user.o

user.img: user.o 
	aarch64-unknown-linux-gnu-ld user.o -T link.ld -o user.elf
	aarch64-unknown-linux-gnu-objcopy -O binary user.elf user.img

clean:
	rm user.elf *.o >/dev/null 2>/dev/null || true
