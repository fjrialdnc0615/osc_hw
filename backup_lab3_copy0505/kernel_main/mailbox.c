#include "gpio.h"
#include "uart.h"
/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/* a properly aligned buffer */
extern volatile unsigned int mbox[36];

#define MBOX_REQUEST    0
//https://github.com/raspberrypi/firmware/wiki/Mailboxes

/* channels */
// Messages sent on this channel can request the platform to power on or off, or to reset the system.
#define MBOX_CH_POWER   0
// Messages sent on this channel can be used to configure the framebuffer's display parameters and to read or write framebuffer data
#define MBOX_CH_FB      1
// Messages sent on this channel can be used to read or write data to the UART, which can be useful for debugging purposes.
#define MBOX_CH_VUART   2
// Messages sent on this channel can be used to perform inter-process communication between different processes running on the platform.
#define MBOX_CH_VCHIQ   3
// Messages sent on this channel can be used to turn on or off specific LEDs or to set the brightness of the LEDs.
#define MBOX_CH_LEDS    4
// Messages sent on this channel can be used to request the state of specific buttons or to receive notifications when a button is pressed.
#define MBOX_CH_BTNS    5
// Messages sent on this channel can be used to read touch coordinates from the touchscreen or to configure the touchscreen's behavior.
#define MBOX_CH_TOUCH   6
// Messages sent on this channel can be used to read performance metrics such as CPU usage, memory usage, and disk I/O.
#define MBOX_CH_COUNT   7
// Messages sent on this channel can be used to request information about the platform's configuration, such as its serial number, or to request firmware updates.
#define MBOX_CH_PROP    8

/* tags */
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_GETBOARDREVISION 0x10002
#define MBOX_TAG_GETSIZEANDADDRESS 0x10005
#define MBOX_TAG_LAST           0


#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */

int mbox_call(unsigned char ch);

void get_board_revision()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETBOARDREVISION ;   // get serial number command
    mbox[3] = 4;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer

    mbox[6] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("board revision is: ");
	uart_hex(mbox[5]);
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

void get_size_and_memory_address()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSIZEANDADDRESS  ;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
        uart_puts("Arm memory address is: ");
        uart_hex(mbox[6]); //hex
	uart_puts("memory size: ");
	uart_hex(mbox[5]); //hex
    } else {
        uart_puts("Unable to query serial!\n");
    }
}

//it takes the channel identifier.
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}
