.section ".text.boot"

.global _start
_start:
	svc  3107
1:
	b 1b
