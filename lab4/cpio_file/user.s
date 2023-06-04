.section ".text.boot"
.global _start
_start:
	nop
	svc  3107
	
	//ldr x0, =0x3F215040
	//mov x1, 65
	//str x1, [x0]

1:
	nop
	b 1b
