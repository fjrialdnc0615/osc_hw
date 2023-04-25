/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

.section ".text.boot"

.global _start

_start:
	ldr	x1, =0x80000
	ldr	x2, =_start
	ldr 	x3, =loader_size //bootloader size

1:	ldr	x4, [x1], #8 //store value from x1 to x4 than mov x1
	str	x4, [x2], #8 //store value from x2 to x4 than mov x2
	sub	w3, w3, #1 //substract size with 1(already divided by 8 in linker)
	cbnz	w3, 1b	// if none zero than mov back to 1

	
	ldr	x1, =_start 
	mov	sp, x1	//set stack pointer before our code

	//clear bss
	ldr	x1, =bss_start
	ldr	w2, =bss_size

2:	cbz	w2, 3f
	str	xzr, [x1], #8 //set x1 location(bss address) to 0 then mov
	sub	w2, w2, #1
	cbnz	w2, 2b

3:	bl	main-0x20000 //60000 <-- main --> 80000 <-- ori main(will be replaced) --> ...
	b	1b
	
