import serial
import os
import os.path
import sys


s = serial.Serial("/dev/tty.usbserial-0001",baudrate=115200)

size = os.stat("kernel8_main.img").st_size
size_tobyte = size.to_bytes(4, 'little')
size_str = "".join(map(lambda x:f"{x:#04x}"[2:],size_tobyte))
print(size, size_str)

print(size_tobyte)
s.write(size_tobyte)


def readline(s):
    size_string = ""
    while True:
        c = s.read().decode()
        print(c)
        if c == '\r':
            continue
        if c == '\n':
            break
        size_string += c
    return size_string

#receive getting size message
receive_string = readline(s)
print(receive_string)

#write the kernel8.img to uart
with open("kernel8_main.img","rb") as f:
    s.write(f.read())

#receive getting file message
receive_string = readline(s)
print(receive_string)
