/*
platform-dependent functions
when porting this is the only code to be rewritten
*/

#ifndef PLATFORM_H
#define	PLATFORM_H

#define PLATFORM "PIC32MX1xx/2xx"

#include "../generic/time_t.h"

/* macro for bitwise operations */
#ifndef BIT
#define BIT(x) (1ul<<(x))
#endif

#define FLASH_PAGE (1024)           /* erasable flash page size in bytes */

#define NVM_SIZE (45*FLASH_PAGE)    /* size in bytes of the reserved NVM storage */
                                    /* in this port the value also determines the size of required FANF memory in RAM */

/* system variables structure */
typedef struct {
    unsigned long key1; /* validity check key */
    unsigned long pC;   /* stored pCode */
    unsigned int PageLines;
    unsigned int ConsoleBaudrate;

    /* other things can be stored here */

    unsigned long key2; /* validity check key */
} __attribute__ ((packed)) sysvars_t;

extern sysvars_t sysvars;           /* system variables */

#define	CLOCK_FREQ (40000000ul)     /* default system clock; note that the peripheral clock has the same rate */
extern unsigned long currentCCLK;   /* current CPU clock frequency in Hz */

#define INTRATE 9600                /* default service interrupt rate in Hz */
extern unsigned int intrate;        /* current service interrupt rate in Hz */

/* default console-related settings */
#define CONSOLE_BAUDRATE    (38400)
#define CONSOLE_RX_BIT      (BIT(4))    /* in port A */
#define CONSOLE_TX_BIT      (BIT(4))    /* in port B */

int console_baudrate;           /* actual console baudrate */

#define getch() _mon_getc(1)

#define UART2_RX_BUFFER 192         /* UART reception buffer size */
extern unsigned char u2_rx[UART2_RX_BUFFER];
extern unsigned short u2_rx_in;     /* incoming character index */
extern unsigned short u2_rx_out;    /* outgoing character index */

extern char ptnlen;                 /* pre-calculated the number of bits in the PMO pattern */

extern volatile unsigned int tick;  /* free running 10us tick counter */
extern volatile date_time_t dt;     /* date/time structure */

#define MKFLG_MOUNTED   (BIT(0))    /* a SD card is currently mounted */

extern unsigned char mkflags;       /* additional run-time flags (see MKFLG_xxx constants) */


/*
low level hardware initialisation
NOTE: the name of this function must not be changed
*/
void platform_init(void);

/*
store the library into file
NOTE: the name of this function must not be changed
*/
void _fanf_store(void);

/*
restore the library from file
NOTE: the name of this function must not be changed
*/
void _fanf_restore(void);

/*
initialise the console port
*/
void console_init(void);

/*
output
kbhit() return 1 if there is a received but unread character, 0 otherwise
*/
int kbhit(void);

/* ========================================================================================================================= */
/* required basic I/O helper functions */
/* ========================================================================================================================= */

/*
input
(blocking) non-zero indicates that the function much be blocking, 0 otherwise
output
getch() return the current character from the input buffer, or EOF in case the buffer is empty
*/
int __attribute__((used)) _mon_getc(int blocking);

/*
write a character to the console
input
(ch) character
*/
void __attribute__((used)) _mon_putc(char ch);

/*
write a string to the console
input
(s) string
*/
void __attribute__((used)) _mon_puts(const char *s);

/*
write to the console
input
(s) data
(count) number of bytes
*/
void __attribute__((used)) _mon_write (const char *s, unsigned int count);

#endif
