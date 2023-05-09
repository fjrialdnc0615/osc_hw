SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -Iinclude -fno-stack-protector -Wall -Wextra -Wpedantic -Werror -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles 
CFLAGS += -I/usr/local/Cellar/aarch64-unknown-linux-gnu/11.2.0/toolchain/lib/gcc/aarch64-unknown-linux-gnu/11.2.0/include
all: kernel8.img

start.o: start.S
	aarch64-unknown-linux-gnu-gcc $(CFLAGS) -c start.S -o start.o

%.o: %.c
	aarch64-unknown-linux-gnu-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-unknown-linux-gnu-ld start.o $(OBJS) -T link.ld -o kernel8.elf
	aarch64-unknown-linux-gnu-objcopy -O binary kernel8.elf kernel8.img
clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb  -display none  -kernel kernel8.img
test:
	qemu-system-aarch64 -M raspi3b -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb  -display none  -kernel kernel8.img -machine dumpdtb=dtb.dtb 
