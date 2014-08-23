#include <xc.h>

#define _SUPPRESS_PLIB_WARNING
#include <plib.h>

#include "platform.h"
#include "mk_words.h"
#include "../generic/fanf.h"

/* configuration fuses for PIC32MX1xx */
#pragma config PMDL1WAY = OFF   /* allow multiple reconfigurations for PMD */
#pragma config IOL1WAY = OFF    /* allow multiple reconfigurations for PPS */
#pragma config FUSBIDIO = OFF   /* USB-related: doesn't matter here */
#pragma config FVBUSONIO = OFF  /* USB-related: doesn't matter here */
#pragma config FPLLIDIV = DIV_2 /* 2:1 input divider for PLL */
#pragma config FPLLMUL = MUL_20 /* 1:20 multiplier for PLL */
#pragma config FPLLODIV = DIV_2 /* 2:1 output divider for PLL */
#pragma config FNOSC = FRCPLL   /* 'FastRC will PLL' oscillator */
#pragma config FSOSCEN = OFF    /* secondary oscillator disabled */
#pragma config IESO = OFF       /* internal/external switchover enabled */
#pragma config POSCMOD = OFF    /* primary oscillator is off (using the internal FRC) */
#pragma config OSCIOFNC = OFF   /* OSCO pin disabled */
#pragma config FPBDIV = DIV_1   /* 1:1 peripheral clock divider */
#pragma config FCKSM = CSECME   /* clock switch enabled, FSCM enabled */
#pragma config WDTPS = PS512    /* 64x watchdog period (~1s) */
#pragma config WINDIS = OFF     /* watchdog in non-window mode */
#pragma config FWDTEN = OFF     /* watchdog controlled by SWDTEN bit */
#pragma config FWDTWINSZ = WINSZ_50 /* watchdog window 50% (doesn't matter as the watchdog is in non-window mode) */
#pragma config DEBUG = OFF      /* debug enabled */
#pragma config JTAGEN = OFF     /* JTAG disabled */
#pragma config ICESEL = ICS_PGx4 /* PGEC4/PGED4 programming pins */
#pragma config PWP = OFF        /* program flash write-protect disabled */
#pragma config BWP = OFF        /* boot flash write-protect disabled */
#pragma config CP = OFF         /* code protection disabled */

unsigned long currentCCLK;      /* current CPU clock frequency in Hz */
unsigned int intrate;           /* current service interrupt rate in Hz */

#define _UART1                  /* console port (used by _mon_putc() and _mon_getc() functions) */

#define CONSOLE_RX_BUFFER 64    /* console reception buffer size */
char console_rx[CONSOLE_RX_BUFFER];
unsigned short console_rx_in=0;     /* incoming character index */
unsigned short console_rx_out=0;    /* outgoing character index */

char ptnlen=8*sizeof(port[0].pattern);      /* pre-calculated the number of bits in the PMO pattern */

unsigned char u2_rx[UART2_RX_BUFFER];
unsigned short u2_rx_in=0;      /* incoming character index */
unsigned short u2_rx_out=0;     /* outgoing character index */

volatile unsigned int tick=0;   /* free running 1us tick counter */
volatile date_time_t dt;        /* date/time structure */

/*
macro to reserve flash memory for saving/loading data and initialise to 0xFF's (thanks to Geoff Graham for this piece of code)
note that (bytes) need to be a multiple of:
- BYTE_PAGE_SIZE (and aligned that way) if you intend to erase
- BYTE_ROW_SIZE (and aligned that way) if you intend to write rows
- sizeof(int) if you intend to write words
*/
#define NVM_ALLOCATE(name,align,bytes) unsigned char name[(bytes)] __attribute__ ((aligned(align),space(prog),section(".nvm"))) = {[0 ...(bytes)-1]=0xff}

NVM_ALLOCATE(nvm_data,FLASH_PAGE,NVM_SIZE);     /* NVM storage area for the library */
NVM_ALLOCATE(nvm_vars,FLASH_PAGE,FLASH_PAGE);   /* NVM storage area for system variables */

#define VALID_KEY 0x601dbead

sysvars_t sysvars;
int console_baudrate=CONSOLE_BAUDRATE;


/*
 handle CPU exceptions
*/
void __attribute__((nomips16)) _general_exception_handler(void)	{
    const static char *szException[] = {
        "Interrupt",                        // 0
        "Unknown",                          // 1
        "Unknown",                          // 2
        "Unknown",                          // 3
        "Address error (load or ifetch)",   // 4
        "Address error (store)",            // 5
        "Bus error (ifetch)",               // 6
        "Bus error (load/store)",           // 7
        "Syscall",                          // 8
        "Breakpoint",                       // 9
        "Reserved instruction",             // 10
        "Coprocessor unusable",             // 11
        "Arithmetic overflow",              // 12
        "Trap (possible divide by zero)",   // 13
        "Unknown",                          // 14
        "Unknown",                          // 15
        "Implementation specific 1",        // 16
        "CorExtend Unuseable",              // 17
        "Coprocessor 2"                     // 18
    };
    volatile static unsigned int codeException;
    volatile static unsigned int addressException;
    const char *pszExcept;
    asm volatile ("mfc0 %0,$13" : "=r" (codeException));
    asm volatile ("mfc0 %0,$14" : "=r" (addressException));
    codeException=(codeException&0x7c)>>2;
    if(codeException<19) {
        pszExcept=szException[codeException];
        printf("\r\n\nCPU EXCEPTION: '%s' at address $%04lx\r\nRestarting...\r\n\n\n",pszExcept,addressException);
        int t=console_baudrate*10;
        while(t--) asm volatile ("nop");    /* just a little delay... */
    }
    SoftReset();
}


/*
initialise the console port
*/
void console_init(void) {
    setbuf(stdin,NULL);
    setbuf(stdout,NULL);
    /* only changing those bits which differ from the default values */
    INTEnable(INT_SOURCE_UART_RX(UART1), INT_DISABLED);
    U1MODE=0;
    U1STA=0;
    U1MODEbits.ON=1;    /* enable UART */
    U1MODEbits.SIDL=1;  /* stop in idle mode */
    U1MODEbits.BRGH=1;  /* use divider for high baudrates */
    U1STAbits.URXEN=1;  /* enable RX */
    U1STAbits.UTXEN=1;  /* enable TX */
    U1BRG=(currentCCLK/4/console_baudrate)-1;   /* set console baudrate */
    TRISA|=CONSOLE_RX_BIT;
    CNPUA|=CONSOLE_RX_BIT;
    PPSInput(3,U1RX,RPA4);      /* set RPA4 as console RX line */
    TRISB&=~CONSOLE_TX_BIT;
    CNPUB|=CONSOLE_TX_BIT;
    PPSOutput(1,RPB4,U1TX);     /* set RPB4 as console TX line */
    console_rx_in=console_rx_out=0;
    INTEnable(INT_SOURCE_UART_RX(UART1), INT_ENABLED);
    INTSetVectorPriority(INT_VECTOR_UART(UART1), INT_PRIORITY_LEVEL_4);
}


/*
 initialise the needed system timer(s)
*/
void timer_init(void) {

    /* initialise Timer2/3 as the main tick timer */
    OpenTimer23(T23_ON | T23_IDLE_STOP | T23_SOURCE_INT | T23_PS_1_1, (currentCCLK/intrate));
    ConfigIntTimer23(T23_INT_ON | T23_INT_PRIOR_1);

    /* initialise Timer4 as clock interrupt with 1ms interval */
    OpenTimer4(T4_ON | T4_IDLE_STOP | T4_PS_1_16, (currentCCLK/1000/16));
    ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_2);

    /* initialise Timer5 as a free running 10us tick counter */
    OpenTimer5(T5_ON | T5_IDLE_STOP | T5_PS_1_1, (currentCCLK/100000));
    ConfigIntTimer5(T5_INT_ON | T5_INT_PRIOR_6);
    tick=0;
}


/*
 initialise the ADC
*/
void adc_init(void) {
    /* only changing those bits which differ from the default values */
    AD1CON1bits.ON=0;
    AD1CON1bits.SIDL=1; /* stop in idle mode */
    AD1CON1bits.SSRC=7; /* internal counter takes care of sampling */
    AD1CON3bits.ADRC=1; /* use FRC clock */
    AD1CON3bits.SAMC=12; /* number of auto-samples in a conversion */
    AD1CSSL=0x1fff;     /* select for scanning all 13 ADC inputs */
    AD1CON1bits.ON=1;   /* start the ADC */
}


/*
low level hardware initialisation
*/
void platform_init(void) {

    /* disable JTAG */
    mJTAGPortEnable(0);

    /* initialise all ports (as digital inputs) */
    TRISA=TRISB=TRISC=0xffffffff;
    LATA=LATB=LATC=0;
    PORTA=PORTB=PORTC=0;
    ANSELA=ANSELB=ANSELC=0;
    CNCONA=CNCONB=CNCONC=0;
    CNENA=CNENB=CNENC=0;    
    CNPUA=CNPUB=CNPUC=0;
    CNPDA=CNPDB=CNPDC=0;

    /* reset all PPS registers */
    RPA0R=RPB3R=RPB4R=RPB15R=RPB7R=RPC7R=0;
    RPC0R=RPC5R=RPA1R=RPB5R=RPB1R=RPB11R=0;
    RPB8R=RPA8R=RPC8R=RPA9R=RPA2R=RPB6R=RPA4R=0;
    RPB13R=RPB2R=RPC6R=RPC1R=RPC3R=RPA3R=0;
    RPB14R=RPB0R=RPB10R=RPB9R=RPC9R=RPC2R=RPC4R=0;
    INT1R=INT2R=INT3R=INT4R=T2CKR=T3CKR=0x0f;
    T4CKR=T5CKR=IC1R=IC2R=IC3R=IC4R=IC5R=0x0f;
    OCFAR=OCFBR=U1RXR=U1CTSR=U2RXR=U2CTSR=0x0f;
    SDI1R=SS1R=SDI2R=SS2R=REFCLKIR=0x0f;

    /* configure system clocks */
    intrate=INTRATE;
    currentCCLK=CLOCK_FREQ;
    SYSTEMConfigPerformance(currentCCLK);
    mOSCSetPBDIV(OSC_PB_DIV_1);     /* the peripheral bus will run at the same clock rate */

    memcpy(&sysvars,&nvm_vars,sizeof(sysvars)); /* load the system variables from NVM */
    memset(port,0,sizeof(port));    /* initialise the ports */
    memset((date_time_t *)&dt,0,sizeof(date_time_t));
    dt.day=1;
    dt.month=1;
    dt.year=2015;
    packTime((date_time_t *)&dt);
    /* dt.valid=VALID_TIME; */

    /* configure the console and other hardware */
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    console_init();
    timer_init();
    adc_init();

    /* enable interrupts globally */
    INTEnableInterrupts();
}


/*
write a character to the console
input
(ch) character
*/
void __attribute__((used)) _mon_putc(char ch) {
    while(!U1STAbits.TRMT);
    U1TXREG=ch;
}


/*
write a string to the console
input
(s) string
*/
void __attribute__((used)) _mon_puts(const char *s) {
    while(s && *s) _mon_putc(*(s++));
}


/*
write to the console
input
(s) data
(count) number of bytes
*/
void __attribute__((used)) _mon_write (const char *s, unsigned int count) {
    while(count--) _mon_putc(*(s++));
}


/*
input
(blocking) non-zero indicates that the function much be blocking, 0 otherwise
output
getch() return the current character from the input buffer, or EOF in case the buffer is empty
*/
int __attribute__((used)) _mon_getc(int blocking) {
    while(console_rx_in==console_rx_out) {
        if(!blocking) return EOF;
    }
    char ch=console_rx[console_rx_out++];
    if(console_rx_out>=CONSOLE_RX_BUFFER) console_rx_out=0;
    return (int)ch;
}


/*
needed because conio.h is not a standard library
output
kbhit() return 1 if there is a received but unread character, 0 otherwise
*/
int kbhit(void) {
    return (console_rx_in!=console_rx_out);
}


/*
store the library into file
NOTE: the name of this function must not be changed
*/
void _fanf_store(void) {
    fanf_mpf(pCode,MPF_CLEANUP);
    const unsigned char *p=nvm_data;
    while((p-nvm_data)<NVM_SIZE) {  /* erase the NVM storage area */
        if(NVMErasePage((unsigned char *)p)) {
            NVMClearError();
            longjmp(shell,ERROR_STORAGE);
        }
        p+=FLASH_PAGE;
    }
    p=nvm_data;
    unsigned char *f=Fmem;
    unsigned int sd, data, chk;
    memcpy((unsigned char *)&sd,(unsigned char *)pCode,4);
    memset((unsigned char *)pCode,code[CODE_NOP],4);
    while(f<pCode) {    /* write FANF memory into NVM storage area */
        memcpy((unsigned char *)&data,(unsigned char *)f,4);
        if(NVMWriteWord((unsigned char *)p,data)) {
            NVMClearError();
            memcpy((unsigned char *)pCode,(unsigned char *)&sd,4);
            longjmp(shell,ERROR_STORAGE);
        }
        memcpy((unsigned char *)&chk,(unsigned char *)p,4);
        if(data!=chk) longjmp(shell,ERROR_STORAGE);
        p+=4;   /* words are 4 bytes in PIC32 */
        f+=4;
    }
    memcpy((unsigned char *)pCode,(unsigned char *)&sd,4);
    sysvars.key1=VALID_KEY;
    sysvars.pC=pCode-Fmem;
    sysvars.key2=VALID_KEY;
    p=nvm_vars;
    f=(unsigned char *)&sysvars;
    while((f-(unsigned char *)&sysvars)<sizeof(sysvars)) {  /* write configuration into NVM storage area */
        memcpy((unsigned char *)&data,(unsigned char *)f,4);
        if(NVMWriteWord((unsigned char *)p,data)) {
            NVMClearError();
            longjmp(shell,ERROR_STORAGE);
        }
        memcpy((unsigned char *)&chk,(unsigned char *)p,4);
        if(data!=chk) longjmp(shell,ERROR_STORAGE);
        p+=4;   /* words are 4 bytes in PIC32 */
        f+=4;
    }
}


/*
restore the library from file
NOTE: the name of this function must not be changed
*/
void _fanf_restore(void) {
    memcpy(&sysvars,(const unsigned char *)nvm_vars,sizeof(sysvars));
    if(sysvars.key1==VALID_KEY && sysvars.key2==VALID_KEY) {
        memcpy(Fmem,(const unsigned char *)nvm_data,FMEM_SIZE);
        pCode=Fmem+sysvars.pC;
        pData=&Fmem[FMEM_SIZE];
        flags=0;
        *ns=0;
        fanf_mpf(pCode,MPF_CLEANUP);
        lines_before_pause=sysvars.PageLines;
        console_baudrate=sysvars.ConsoleBaudrate;
        console_init();
    }
}


/*
UART1 (console) interrupt handler
*/
void __ISR(_UART1_VECTOR, ipl4) IntUart1Handler(void) {
	if(INTGetFlag(INT_SOURCE_UART_RX(UART1))) {
        while(UARTReceivedDataIsAvailable(UART1)) {
            char c=U1RXREG;
            char e=(UARTGetLineStatus(UART1)&0b1110);
            U1STA&=~0b1110;
            if(!e) {
                console_rx[console_rx_in++]=c;
                if(console_rx_in>=CONSOLE_RX_BUFFER) console_rx_in=0;
                if(c=='\r') {
                    console_rx[console_rx_in++]='\n';
                    if(console_rx_in>=CONSOLE_RX_BUFFER) console_rx_in=0;
                    printf("\n");
                }
            }
        }
        INTClearFlag(INT_SOURCE_UART_RX(UART1));
    }
}


/*
UART2 RX interrupt function
*/
void __ISR(_UART2_VECTOR, ipl3) IntUart2Handler(void) {
    if(INTGetFlag(INT_SOURCE_UART_RX(UART2))) {
        while(UARTReceivedDataIsAvailable(UART2)) {
            char c=UARTGetDataByte(UART2);
            char e=(UARTGetLineStatus(UART2)&0b1110);
            U2STA&=~0b1110;
            if(!e) {
                u2_rx[u2_rx_in++]=c;
                if(u2_rx_in>=UART2_RX_BUFFER) u2_rx_in=0;
            }
        }
        INTClearFlag(INT_SOURCE_UART_RX(UART2));
    }
}


/*
port handling timer interrupt function
every call serves one port only
*/
void __ISR( _TIMER_23_VECTOR, ipl1) T23Interrupt(void) {
    static signed char adcp=-1;    
    if(adcp>=0 && AD1CON1bits.DONE) {   /* ADC has finished with conversion */
        port[adcp].ain=ADC1BUF0;
        adcp=-1;    /* release the ADC port for a new assignment */
    }
    unsigned char n=0;
    do {
        unsigned char p=!!(*(portdef[n].base+PORT)&BIT(portdef[n].pbit));   /* get the current logic value from the port */
        switch(port[n].func&PFN_MASK) {
            default: break;
            case PFN_DIN: {     /* count and measure frequency on DIN ports */
                port[n].fqc++;
                if((p-port[n].last)==1) {       /* 0-1 transition */
                    if((port[n].func&PFN_ANY) || !(port[n].func&PFN_NEG)) port[n].counter++;
                    port[n].tlcc=0;
                }
                else if((p-port[n].last)==-1) { /* 1-0 transition */
                    if((port[n].func&PFN_ANY) || (port[n].func&PFN_NEG)) port[n].counter++;
                    port[n].freq=(port[n].freq+port[n].fqc)/2;
                    port[n].fqc=0;
                    port[n].tlcc=0;
                }
                else port[n].tlcc++;
                break;
            }
            case PFN_AIN: {     /* initiate ADC reading for AIN ports */
                if(adcp<0) {
                    adcp=n;     /* assign the ADC to this port */
                    AD1CHSbits.CH0SA=portdef[n].adcchn;
                    AD1CON1bits.SAMP=1; /* start sampling */
                }
                break;
            }
            case PFN_PMO:
            case PFN_PWM: {   /* shift the pattern out on the output for PMO and PWM ports */
                if(++port[n].odivcntr>=port[n].odiv) {
                    port[n].odivcntr=0;
                    if(--port[n].onbit<0) {
                        port[n].onbit=(ptnlen-1);
                        if(port[n].func&PFN_PMO1) { /* check for once-off PMO condition */
                            if(port[n].func&PFN_NEG) port[n].pattern=~0; else port[n].pattern=0;
                        }
                    }
                    if(port[n].pattern & BIT(port[n].onbit))
                        *(portdef[n].base+LATSET)=BIT(portdef[n].pbit);
                    else
                        *(portdef[n].base+LATCLR)=BIT(portdef[n].pbit);                    
                }
                break;
            }
        }
        port[n].last=p;
    } while(++n<PORTS);
    mT23ClearIntFlag();
}


/*
1-millisecond clock interrupt function
*/
void __ISR( _TIMER_4_VECTOR, ipl2) T4Interrupt(void) {
    clockTime((date_time_t *)&dt,1000);
    disk_timerproc();
    mT4ClearIntFlag();
}


/*
10-microsecond tick interrupt function
*/
void __ISR( _TIMER_5_VECTOR, ipl6) T5Interrupt(void) {
    tick++;
    mT5ClearIntFlag();
}
