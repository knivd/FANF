/*
platform-dependent word definitions and functions
*/

#if defined(MORE_WORDS_DEF)
/* word definitions (NOTHING ELSE HERE!) */
/* {"someword",    W_EXTRA, 0x3e,   1,  f_someword},    // sample definition in this section */

{"option",  W_EXTRA,    0xc0,   1,  f_option},      /* v p option */
                                                    /* set shell/device option p with value v */
                                                    /* valid strings for p (case sensitive): */
                                                    /* "PageLines" define the number of printed lines in terminal before pause (0: disable) */
                                                    /* "ConsoleBaudrate" define the baudrate for the console (default is 38400) */
                                                    /*        valid choices are: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 */

{"u10tick", W_EXTRA,    0xc1,   1,  f_u10tick},     /* u10tick */
                                                    /* return in stack the current value read from the 10-microsecond tick counter */

{"cpuclk",  W_EXTRA,    0xc2,   1,  f_cpuclk},      /* f cpuclk */
                                                    /* select CPU clock frequency */
                                                    /* selectable values are 1, 6, 12, 24, 32, 40 (default), 48 */
                                                    /* console and timers are re-initialised automatically, but for other peripherals that must be done by the user */

{"portcfg", W_EXTRA,    0xd0,   1,  f_portcfg},     /* n func[modifiers] portcfg */
                                                    /* configure port n for function func (func is supplied as text) */
                                                    /* functions: */
                                                    /* DIN (digital input), DOUT (digital output), AIN (analogue input) */
                                                    /* PMO (pattern modulated output) */
                                                    /* PWM (pulse width modulated output) */
                                                    /* optional modifiers are part of the function text */
                                                    /* modifiers: */
                                                    /* 1 (pull-up), 0 (pull-down), * (open-drain output), ! (callback generating input) */
                                                    /* - (action triggered on 1-0 transition; for PMO sets the exit state of the portin once-off events) */
                                                    /* = (action triggered on either 0-1 or 1-0 transition) */
                                                    /* ! (applicable to PMO only; specify once-off operation) */

{"whatcfg", W_EXTRA,    0xd1,   1,  f_whatcfg},     /* n whatcfg */
                                                    /* return the possible configuration for a port */
                                                    /* see PFN_xxx constants for details */

{"portrd",  W_EXTRA,    0xd2,   1,  f_portrd},      /* n portrd */
                                                    /* read port n and return its current digital or analogue value according to its configuration */
                                                    /* behaviour depends on the port configuration: */
                                                    /* DIN -> the actual port logic value 0 or 1 */
                                                    /* DOUT -> the last written to the port logic value 0 or 1 */
                                                    /* AIN -> measured voltage on the port */
                                                    /* PMO -> PMO pattern value */
                                                    /* PWM -> PWM value */

{"portwr",  W_EXTRA,    0xd3,   1,  f_portwr},      /* n v portwr */
                                                    /* write v into port n  */
                                                    /* behaviour depends on the port configuration: */
                                                    /* DIN -> set initial value in the port counter */
                                                    /* DOUT -> set the digital output with logic value 0 or 1 */
                                                    /* AIN -> no effect */
                                                    /* PMO -> set new PMO pattern value */
                                                    /* PWM -> set new PWM value (0..32) */

{"portfq",  W_EXTRA,    0xd4,   1,  f_portfq},      /* n portfq */
                                                    /* return the currently measured frequency [Hz] on a DIN port */

{"portcn",  W_EXTRA,    0xd5,   1,  f_portcn},      /* n portcn */
                                                    /* return the current counter value on a DIN port */
                                                    /* counter works as per the triggering action specified in portcfg (default: 0-1 transition) */

{"porttlc", W_EXTRA,    0xd6,   1,  f_porttlc},     /* n porttlc */
                                                    /* return the elapsed time since the last detected change on a DIN port */
                                                    /* the returned time is in microseconds, which limits the maximum detected interval at 4295 seconds */

{"portdiv", W_EXTRA,    0xd7,   1,  f_portdiv},     /* n portdiv */
                                                    /* set output divider for PMO and PWM */
                                                    /* value 0 has the same effect as value 1; negative values have no effect */

{"pconfig", W_EXTRA,    0xdd,   1,  f_pconfig},     /* c p b pconfig */
                                                    /* configure communication port c with protocol p and baudrate b */
                                                    /* the same word is used to close the communication port as well */
                                                    /* executing with baudrate 0 release the assigned hardware port */
                                                    /* supported peripheral devices: */
                                                    /* 1: UART, 5: SPI, 8: I2C */

{"pout",    W_EXTRA,    0xde,	1,  f_pout},        /* d x pout */
                                                    /* (top) ---> (peripheral device d) */
                                                    /* text data is output as it is while number type is converted into text and then output */
                                                    /* if the stack is empty, nothing is output */
                                                    /* will return the number of actually sent characters */
                                                    /* supported peripheral devices: */
                                                    /* 1: UART, 5: SPI, 8: I2C */

{"pin",     W_EXTRA,    0xdf,	1,  f_pin},         /* d x pin */
                                                    /* (x readings from peripheral device d) ---> (top) */
                                                    /* read exactly x characters from p into a text type element */
                                                    /* supported peripheral devices: */
                                                    /* 1: UART, 5: SPI, 8: I2C */

{"strgerr", W_EXTRA,    0xe0,   1,  f_strgerr},     /* strgerr */
                                                    /* return into stack the execution result of the last storage operation */
                                                    /* value 0 is no error */

{"mount",   W_EXTRA,    0xe1,   1,  f_mountf},      /* mount */
                                                    /* mount a SD card in the Microkite module */

{"unmount", W_EXTRA,    0xe2,   1,  f_unmountf},    /* unmount */
                                                    /* unmount already mounted SD card and release the SPI hardware */

{"makefs",  W_EXTRA,    0xe3,   1,  f_makefs},      /* makefs */
                                                    /* initialise new file system */
                                                    /* this word will destroy all current information on the SD card */

{"dir",     W_EXTRA,    0xe4,   1,  f_dirf},        /* [n] dir */
                                                    /* show files and sub-directories starting from path n */
                                                    /* if the stack is empty, will assume n as the current path */

{"mkdir",   W_EXTRA,    0xe5,   1,  f_mkdirf},      /* n mkdir */
                                                    /* make directory with name n */

{"chdir",   W_EXTRA,    0xe6,   1,  f_chdirf},      /* n chdir */
                                                    /* change the current path to directory with name n */

{"fcopy",   W_EXTRA,    0xe7,   1,  f_fcopyf},      /* n f fcopy */
                                                    /* copy file with name n to f; return 1 on success and 0 otherwise */

{"fdel",	W_EXTRA,    0xe8,	1,  f_fdelf},       /* n fdel */
                                                    /* delete file with name n */

{"fren",	W_EXTRA,    0xe9,	1,  f_frenf},       /* n f fren */
                                                    /* change a file name from n to f */

#elif defined(MORE_WORDS_ERR)
/* error messages as 'case' statements (NOTHING ELSE HERE!) */
/* case ERROR_CODE: printf("error message"); break;     // sample definition in this section */

case ERROR_INV_PORT_NUM: printf("invalid port number"); break;
case ERROR_INV_PORT_FUNC: printf("invalid port function"); break;
case ERROR_PORT_FUNC_ASSGN: printf("port function assigned to hardware"); break;
case ERROR_NOT_INITIALISED: printf("device not initialised"); break;
case ERROR_UNKNOWN_DEVICE: printf("unknown device"); break;
case ERROR_INVALID_OPTION: printf("invalid option"); break;

#else

/* function definitions */
/* void f_someword(void);   // sample definition in this section */

#ifndef MK_WORDS_H
#define	MK_WORDS_H

#include <xc.h>

/* port function flags (lowest 20 bits in port[].func are reserved for function flags) */
#define PFN_MASK    (0xfffff)   /* general bitmask of the function bits */
#define PFN_DIN     (BIT(0))    /* digital input */
#define PFN_DOUT    (BIT(1))    /* digital output */
#define PFN_AIN     (BIT(2))    /* analogue input */
#define PFN_PMO     (BIT(3))    /* pattern modulated output */
#define PFN_PWM     (BIT(4))    /* pulse width modulation */

/* modifier flags for the ports (highest 12 bits in port[].func are reserved for modifier flags) */
#define PFN_PUP     (BIT(20))   /* internal pull-up */
#define PFN_PDN     (BIT(21))   /* internal pull-down */
#define PFN_OD      (BIT(22))   /* open-drain output */
#define PFN_NEG     (BIT(23))   /* action triggered on 1-0 transition (normally it is 0-1) */
#define PFN_ANY     (BIT(24))   /* action triggered on any transition */
#define PFN_PMO1    (BIT(25))   /* PMO in once-off mode */
#define PFN_FUNC    (BIT(31))   /* special hardware function (such as part of a communication block) */

/* number of ports in the system */
#define PORTS 33

/* base address of the hardware ports */
#define BASEA  ((volatile unsigned int *)0xbf886020)
#define BASEB  ((volatile unsigned int *)0xbf886120)
#define BASEC  ((volatile unsigned int *)0xbf886220)

/* offsets from the base address for the port-associated registers */
#define ANSEL       -8
#define ANSELCLR    -7
#define ANSELSET    -6
#define ANSELINV    -5
#define TRIS        -4
#define TRISCLR     -3
#define TRISSET     -2
#define TRISINV     -1
#define PORT        0
#define PORTCLR     1
#define PORTSET     2
#define PORTINV     3
#define LAT         4
#define LATCLR      5
#define LATSET      6
#define LATINV      7
#define ODC         8
#define ODCCLR      9
#define ODCSET      10
#define ODCINV      11
#define CNPU        12
#define CNPUCLR     13
#define CNPUSET     14
#define CNPUINV     15
#define CNPD        16
#define CNPDCLR     17
#define CNPDSET     18
#define CNPDINV     19
#define CNCON       20
#define CNCONCLR    21
#define CNCONSET    22
#define CNCONINV    23
#define CNEN        24
#define CNENCLR     25
#define CNENSET     26
#define CNENINV     27
#define CNSTAT      28
#define CNSTATCLR   29
#define CNSTATSET   30
#define CNSTATINV   31

/* port structure definition */
typedef struct {
    volatile unsigned int *base;    /* base PORT address */
    unsigned char pbit;             /* bit in the hardware port */
    unsigned char adcchn;           /* ADC channel number (if available, otherwise -1) */
    unsigned long func;             /* bitmask of the possible assignment functions for the port (see PFN_xxx constants) */
} f_port_t;

const f_port_t portdef[PORTS];

/* run-time port structure */
typedef struct {
    unsigned long func;         /* assigned function (can be only one!) */
    unsigned short ain;         /* analogue input ADC value */
    unsigned long freq;         /* frequency counter on input */
    unsigned long counter;      /* current value of the input incremental counter */
    unsigned long tlcc;         /* elapsed time since the last change (in ticks) */
    unsigned long pattern;      /* PMO and PWM pattern */
    unsigned char last;         /* last logic value of the port (used for transition detecting) */
    signed char onbit;          /* current pattern bit number for PMO and PWN */
    unsigned long odiv;         /* divider for PMO and PWM */
    unsigned long odivcntr;     /* working divider counter for PMO and PWM */
    unsigned long fqc;          /* running frequency counter on input */
} r_port_t;

extern r_port_t port[PORTS];

/* added error codes */
#define ERROR_INV_PORT_NUM      100
#define ERROR_INV_PORT_FUNC     101
#define ERROR_PORT_FUNC_ASSGN   102
#define ERROR_NOT_INITIALISED   103
#define ERROR_UNKNOWN_DEVICE    104
#define ERROR_INVALID_OPTION    105

void f_option(void);
void f_u10tick(void);
void f_cpuclk(void);
void f_portcfg(void);
void f_whatcfg(void);
void f_portrd(void);
void f_portwr(void);
void f_portfq(void);
void f_portcn(void);
void f_porttlc(void);
void f_portdiv(void);
void f_pout(void);
void f_pin(void);
void f_pconfig(void);
void f_strgerr(void);
void f_unmountf(void);
void f_mountf(void);
void f_makefs(void);
void f_dirf(void);
void f_mkdirf(void);
void f_chdirf(void);
void f_fcopyf(void);
void f_frenf(void);
void f_fdelf(void);

#endif
#endif
