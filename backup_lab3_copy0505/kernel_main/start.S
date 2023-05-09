#define CORE0_TIMER_IRQ_CTRL 0x40000040
.section ".text.boot"

.global _start

_start:
	//when firmware is activated it will load dtb_ptr addr in x0
    ldr		x1, =_dtb_ptr	// put _dtb_ptr into register1
    str		x0, [x1]		// store dtb address from x0 to _dtb_ptr

    // read cpu id, stop slave cores
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    //cpu id > 0, stop

1:  wfe
    b       1b
2:  // cpu id == 0

	// Switch from EL2 to EL1
    bl from_el2_to_el1
    // the next instruction runs in EL1

	//set exception vector table
	adr x0, exception_vector_table
	//uses the value stored in the VBAR_EL1 register to locate the exception vector table
	msr vbar_el1, x0

	// set top of stack just before our code (stack grows to a lower address per AAPCS64)
    ldr     x1, =_start
    mov     sp, x1

    // clear bss
    ldr     x1, =__bss_start
    ldr     w2, =__bss_size

3:  cbz     w2, 4f
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b

core_timer_enable:
//    mrs x0, cntfrq_el0 //get hz (s/freq)
//	mov x3, 5		   //10 times hz means 10 secs
//	mul x0, x0, x3     //x3 * x0 -> x0
//    msr cntp_tval_el0, x0 // set expired time
    mov x0, 2		   //
    ldr x1, =CORE0_TIMER_IRQ_CTRL //second bit set nCNTHPIRQ IRQ Enabled
    str x0, [x1] // unmask timer interrupt
    mov x0, 1 
//    msr cntp_ctl_el0, x0 //enable timer


    // jump to C code, should not return
4:  bl      main
// for failsafe, halt this core too
    b       1b

.balign 0x800
exception_vector_table:
  b exception_handler // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b irq_exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7
  b exception_handler
  .align 7

from_el2_to_el1:
    mov x0, (1<<31) // 31th bit configure the el1 using aarch64
    msr hcr_el2, x0 //write the values into hcr_el2

    //configures the new level (EL1) to work in a specific mode called "EL1h" and disables interrupts
    mov x0, 0x345
    msr spsr_el2, x0
    msr elr_el2, lr
    eret // return to EL1

// save general registers to stack
.macro save_all
    sub sp, sp, 32 * 8
    stp x0, x1, [sp ,16 * 0]
    stp x2, x3, [sp ,16 * 1]
    stp x4, x5, [sp ,16 * 2]
    stp x6, x7, [sp ,16 * 3]
    stp x8, x9, [sp ,16 * 4]
    stp x10, x11, [sp ,16 * 5]
    stp x12, x13, [sp ,16 * 6]
    stp x14, x15, [sp ,16 * 7]
    stp x16, x17, [sp ,16 * 8]
    stp x18, x19, [sp ,16 * 9]
    stp x20, x21, [sp ,16 * 10]
    stp x22, x23, [sp ,16 * 11]
    stp x24, x25, [sp ,16 * 12]
    stp x26, x27, [sp ,16 * 13]
    stp x28, x29, [sp ,16 * 14]
    str x30, [sp, 16 * 15]
.endm

// load general registers from stack
.macro load_all
    ldp x0, x1, [sp ,16 * 0]
    ldp x2, x3, [sp ,16 * 1]
    ldp x4, x5, [sp ,16 * 2]
    ldp x6, x7, [sp ,16 * 3]
    ldp x8, x9, [sp ,16 * 4]
    ldp x10, x11, [sp ,16 * 5]
    ldp x12, x13, [sp ,16 * 6]
    ldp x14, x15, [sp ,16 * 7]
    ldp x16, x17, [sp ,16 * 8]
    ldp x18, x19, [sp ,16 * 9]
    ldp x20, x21, [sp ,16 * 10]
    ldp x22, x23, [sp ,16 * 11]
    ldp x24, x25, [sp ,16 * 12]
    ldp x26, x27, [sp ,16 * 13]
    ldp x28, x29, [sp ,16 * 14]
    ldr x30, [sp, 16 * 15]
    add sp, sp, 32 * 8
.endm

exception_handler:
    save_all
    bl exception_handler_c
    load_all
    eret

irq_exception_handler:
	save_all
	bl irq_exception_handler_c
	load_all
	eret


.global _dtb_ptr	// define a global variable _dtb_ptr
.section .data		// _dtb_ptr is in data section
_dtb_ptr: .dc.a 0x0	// it defines _dtb_ptr to be a 8-byte constant with a value of 0x0
