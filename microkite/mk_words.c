#include <xc.h>

#define _SUPPRESS_PLIB_WARNING
#include <plib.h>

#include "mk_words.h"
#include "platform.h"
#include "../generic/fanf.h"
#include "fatfs/ff.h"


/* ports are numbered according to the schematic of Microkite */
const f_port_t portdef[PORTS] = {
/* P0 */    { BASEB,  5,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P1 */    { BASEA,  1,  1,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P2 */    { BASEB,  14, 10,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P3 */    { BASEC,  6,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P4 */    { BASEC,  7,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P5 */    { BASEC,  8,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P6 */    { BASEC,  9,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P7 */    { BASEB,  12, 12,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P8 */    { BASEA,  9,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P9 */    { BASEC,  4,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P10 */   { BASEC,  5,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P11 */   { BASEB,  9,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P12 */   { BASEB,  8,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P13 */   { BASEB,  7,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P14 */   { BASEB,  11, -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P15 */   { BASEB,  10, -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P16 */   { BASEB,  3,  5,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P17 */   { BASEB,  6,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P18 */   { BASEA,  3,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P19 */   { BASEA,  2,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P20 */   { BASEB,  0,  2,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P21 */   { BASEB,  1,  3,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P22 */   { BASEB,  2,  4,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P23 */   { BASEB,  15, 9,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P24 */   { BASEB,  13, 11,   (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P25 */   { BASEA,  0,  0,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P26 */   { BASEC,  0,  6,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P27 */   { BASEC,  1,  7,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P28 */   { BASEC,  2,  8,    (PFN_DOUT | PFN_DIN | PFN_AIN | PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P29 */   { BASEA,  8,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P30 */   { BASEA,  10, -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P31 */   { BASEA,  7,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) },
/* P32 */   { BASEC,  3,  -1,   (PFN_DOUT | PFN_DIN |           PFN_PMO | PFN_PWM | PFN_PUP | PFN_PDN | PFN_OD) }
};

r_port_t port[PORTS];       /* run-time port data */
unsigned char mkflags=0;    /* additional run-time flags (see MKFLG_xxx constants) */


void f_portcfg(void) {
    int n=(int)get_num(1);      /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    unsigned char *f, *fs;
    f_data_t *e=get_text(0,&f); /* port definition */
    fs=f;        
    if(!f) longjmp(shell,ERROR_INV_PORT_FUNC);
    if(port[n].func&PFN_FUNC) longjmp(shell,ERROR_PORT_FUNC_ASSGN);
    unsigned long func=0;

    /* reset the port */
    *(portdef[n].base+ANSELCLR)=BIT(portdef[n].pbit);
    *(portdef[n].base+TRISSET)=BIT(portdef[n].pbit);
    *(portdef[n].base+LATCLR)=BIT(portdef[n].pbit);
    *(portdef[n].base+PORTCLR)=BIT(portdef[n].pbit);    
    *(portdef[n].base+CNPUCLR)=BIT(portdef[n].pbit);
    *(portdef[n].base+CNPDCLR)=BIT(portdef[n].pbit);
    *(portdef[n].base+ODCCLR)=BIT(portdef[n].pbit);
    memset(&port[n],0,sizeof(r_port_t));
    port[n].last=!!(*(portdef[n].base+PORT)&BIT(portdef[n].pbit));

    /* set port function */
    if(!strncmp(f,"DIN",3)) {
        if(!(portdef[n].func&PFN_DIN)) longjmp(shell,ERROR_INV_PORT_FUNC);
        func=PFN_DIN;
        f+=3;
    }
    else if(!strncmp(f,"DOUT",4)) {
        if(!(portdef[n].func&PFN_DOUT)) longjmp(shell,ERROR_INV_PORT_FUNC);
        *(portdef[n].base+TRISCLR)=BIT(portdef[n].pbit);
        func=PFN_DOUT;
        f+=4;
    }
    else if(!strncmp(f,"AIN",3)) {
        if(!(portdef[n].func&PFN_AIN)) longjmp(shell,ERROR_INV_PORT_FUNC);
        *(portdef[n].base+ANSELSET)=BIT(portdef[n].pbit);
        func=PFN_AIN;
        f+=3;
    }
    else if(!strncmp(f,"PMO",3)) {
        if(!(portdef[n].func&PFN_PMO)) longjmp(shell,ERROR_INV_PORT_FUNC);
        *(portdef[n].base+TRISCLR)=BIT(portdef[n].pbit);
        func=PFN_PMO;
        f+=3;
    }
    else if(!strncmp(f,"PWM",3)) {
        if(!(portdef[n].func&PFN_PWM)) longjmp(shell,ERROR_INV_PORT_FUNC);
        *(portdef[n].base+TRISCLR)=BIT(portdef[n].pbit);
        func=PFN_PWM;
        f+=3;
    }
    else longjmp(shell,ERROR_INV_PORT_FUNC);

    /* process the modifier flags */
    unsigned long fn=(func&PFN_MASK);
    while(f<(fs+e->len)) {
        if(*f=='1') {
            if(fn==PFN_AIN) longjmp(shell,ERROR_INV_PORT_FUNC); /* no pull-ups allowed for the analogue inputs */
            if(!(portdef[n].func&PFN_PUP)) longjmp(shell,ERROR_INV_PORT_FUNC);
            *(portdef[n].base+CNPUSET)=BIT(portdef[n].pbit);
            func|=PFN_PUP;
        }
        else if(*f=='0') {
            if(fn==PFN_AIN) longjmp(shell,ERROR_INV_PORT_FUNC); /* no pull-downs allowed for the analogue inputs */
            if(!(portdef[n].func&PFN_PDN)) longjmp(shell,ERROR_INV_PORT_FUNC);
            *(portdef[n].base+CNPDSET)=BIT(portdef[n].pbit);
            func|=PFN_PDN;
        }
        else if(*f=='*') {
            if(fn!=PFN_DOUT && fn!=PFN_PMO) longjmp(shell,ERROR_INV_PORT_FUNC); /* only outputs can be open-drain */
            if(!(portdef[n].func&PFN_OD)) longjmp(shell,ERROR_INV_PORT_FUNC);
            *(portdef[n].base+ODCSET)=BIT(portdef[n].pbit);
            func|=PFN_OD;
        }        
        else if(*f=='-') {
            if(fn!=PFN_DIN && fn!=PFN_PMO) longjmp(shell,ERROR_INV_PORT_FUNC);  /* only applicable to digital inputs and PMO */
            func&=~PFN_ANY;
            func|=PFN_NEG;
        }
        else if(*f=='=') {
            if(fn!=PFN_DIN) longjmp(shell,ERROR_INV_PORT_FUNC); /* applicable to digital inputs only */
            func&=~PFN_NEG;
            func|=PFN_ANY;
        }
        else if(*f=='!') {
            if(fn!=PFN_PMO) longjmp(shell,ERROR_INV_PORT_FUNC); /* applicable to PMO only */
            func|=PFN_PMO1;
        }
        else longjmp(shell,ERROR_INV_PORT_FUNC);
        f++;
    }
    port[n].func=func;
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the text parameter */
}


void f_whatcfg(void) {
    int n=(int)get_num(0);  /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    set_num(stid,TYPE_UINT,-1,(REAL)portdef[n].func);
}


void f_portrd(void) {
    int n=(int)get_num(0);      /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    switch(port[n].func&PFN_MASK) {
        default: case PFN_DIN: set_num(stid,TYPE_UINT,-1,(REAL)!!(*(portdef[n].base+PORT)&BIT(portdef[n].pbit))); break;
        case PFN_DOUT: set_num(stid,TYPE_UINT,-1,(REAL)!!(*(portdef[n].base+LAT)&BIT(portdef[n].pbit))); break;
        case PFN_PMO: set_num(stid,TYPE_UINT,-1,(REAL)port[n].pattern); break;
        case PFN_PWM: {            
            unsigned int p=port[n].pattern;
            unsigned int v=0;
            do v+=(p&1); while(p>>=1);
            set_num(stid,TYPE_UINT,-1,(REAL)v); break;
            break;
        }
        case PFN_AIN: {
            REAL v=(3.3/1024)*(REAL)(port[n].ain&0x3ff);
            set_num(stid,TYPE_REAL,-1,v);
            break;
        }
    }
}


void f_portwr(void) {
    unsigned long v=(unsigned long)get_num(0);  /* value */
    int n=(int)get_num(0);  /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    switch(port[n].func&PFN_MASK) {
        default: case PFN_DIN: port[n].counter=v; break;        
        case PFN_PMO: {
            port[n].pattern=v;
            port[n].onbit=(ptnlen-1);
            break;
        }
        case PFN_PWM: {
            port[n].pattern=0;
            if(v>32) v=32;
            while(v--) port[n].pattern=(port[n].pattern<<1)+1;
            break;
        }
        case PFN_DOUT: {
            if(v) *(portdef[n].base+LATSET)=BIT(portdef[n].pbit); else *(portdef[n].base+LATCLR)=BIT(portdef[n].pbit);
            break;
        }
        case PFN_AIN: break;    /* no effect */
    }
}


void f_portfq(void) {
    int n=(int)get_num(0);      /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    if((port[n].func&PFN_MASK)!=PFN_DIN) longjmp(shell,ERROR_INV_PORT_FUNC);    /* only DIN ports can count and measure frequency */
    REAL v, f=port[n].freq;
    if(f) v=(intrate/f); else v=0.0;
    set_num(stid,TYPE_REAL,-1,v);
}


void f_portcn(void) {
    int n=(int)get_num(0);      /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    if((port[n].func&PFN_MASK)!=PFN_DIN) longjmp(shell,ERROR_INV_PORT_FUNC);    /* only DIN ports can count and measure frequency */
    set_num(stid,TYPE_UINT,-1,(REAL)port[n].counter);
}


void f_porttlc(void) {
    int n=(int)get_num(0);      /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    if((port[n].func&PFN_MASK)!=PFN_DIN) longjmp(shell,ERROR_INV_PORT_FUNC);    /* only DIN ports can count and measure frequency */
    REAL t=port[n].tlcc;
    REAL v=(1000000/intrate)*t;
    set_num(stid,TYPE_REAL,-1,v);
}


void f_portdiv(void) {
    unsigned long v=(unsigned long)get_num(0);  /* value */
    int n=(int)get_num(0);  /* port number */
    if(n<0 || n>=PORTS) longjmp(shell,ERROR_INV_PORT_NUM);
    if(v>=0) port[n].odiv=v;
}


/* ========================================================================================================================= */
/* integrating FatFs and functions to use it */

#if _USE_LFN
TCHAR Lfname[256];
#endif

FRESULT FatFsErr=FR_OK;
FATFS FatFs;
FILINFO Finfo;
FIL File[MAX_FILES];


/*
this function provides time in FAT format
valid time must be returned even if the system does not support an RTC
*/
unsigned long get_fattime(void) {
	return(((unsigned long)(dt.year-1980)<<25) | ((unsigned long)dt.month<<21) | ((unsigned long)dt.day<<16) |
           ((unsigned long)(dt.hour<<11)) | ((unsigned long)(dt.minute<<5)) | ((unsigned long)(dt.second>>1)));
}


/*
when executing in direct mode print errors from FatFs
input
(FatFsErr) error code
*/
void fferr(void) {
    if((flags&FLAG_DIRECT) && FatFsErr!=FR_OK) {
        new_line();
        printf("DRIVE ERROR %u: ",FatFsErr);
        switch(FatFsErr) {
            default: break;
            case FR_DISK_ERR: printf("unrecoverable error occured in the media access layer"); break;
            case FR_INT_ERR: printf("internal error in the drive access functions"); break;
            case FR_NOT_READY: printf("device is not ready"); break;
            case FR_NO_FILE: printf("file is not found"); break;
            case FR_NO_PATH: printf("path is not found"); break;
            case FR_INVALID_NAME: printf("invalid filename"); break;
            case FR_DENIED: printf("access denied"); break;
            case FR_EXIST: printf("file already exists"); break;
            case FR_INVALID_OBJECT: printf("invalid file/directory structure"); break;
            case FR_WRITE_PROTECTED: printf("device is write-protected"); break;
            case FR_INVALID_DRIVE: printf("invalid device"); break;
            case FR_NOT_ENABLED: printf("device is not mounted"); break;
            case FR_NO_FILESYSTEM: printf("no valid FAT on the device"); break;
            case FR_MKFS_ABORTED: printf("function aborted"); break;
            case FR_TIMEOUT: printf("timeout error"); break;
            case FR_LOCKED: printf("object is locked for processing"); break;
            case FR_NOT_ENOUGH_CORE: printf("not enough memory for file functions"); break;
            case FR_TOO_MANY_OPEN_FILES: printf("too many open files"); break;
            case FR_INVALID_PARAMETER: printf("invalid parameter in the file functions"); break;
        }
        new_line();
        if(mkflags&MKFLG_MOUNTED) {
            f_unmountf();
            memset(&FatFs,0,sizeof(FATFS));
            f_mountf();
            FatFsErr=FR_OK;
        }
    }
}


void f_strgerr(void) {
    set_num(stid,TYPE_UINT,-1,(REAL)FatFsErr);
}


void f_unmountf(void) {
    if(!(mkflags&MKFLG_MOUNTED)) return;
    FatFsErr=f_mount(0,"",1);   /* force unmount the drive */
    memset(&FatFs,0,sizeof(FATFS));
    *(BASEC+TRISSET)=BIT(3);    /* SD card ~select line */
    *(BASEC+CNPUCLR)=BIT(3);
    mkflags&=~MKFLG_MOUNTED;
    FatFsErr=FR_OK;
}


void f_mountf(void) {
    if(mkflags&MKFLG_MOUNTED) return;
    *(BASEC+CNPUSET)=BIT(3);    /* pull-up on the select line */
    *(BASEC+TRISCLR)=BIT(3);    /* SD card ~select line */
    SPI1CON=0;
    SPI1CON2=0;
    SPI1CONbits.ON=1;       /* enable SPI1 */
    SPI1CONbits.SIDL=1;     /* stop in idle mode */
    SPI1BRG=(currentCCLK/100000)-1; /* 100k */
    SPI1CONbits.CKP=1;      /* SPI mode 3 */
    SPI1CONbits.MSTEN=1;    /* enable master mode */
    SPI1CONbits.SRXISEL=3;
    SPI1STATbits.SPIROV=0;  /* clear the overrun flag */
    *(BASEB+TRISSET)=BIT(5);
    *(BASEA+TRISCLR)=BIT(1);
    *(BASEB+TRISCLR)=BIT(14);
    PPSInput(2,SDI1,RPB5);
    PPSOutput(2,RPA1,SDO1);
    port[0].func|=PFN_FUNC;
    port[1].func|=PFN_FUNC;
    port[2].func|=PFN_FUNC;
    port[32].func|=PFN_FUNC;
    FatFsErr=f_mount(&FatFs,"",1);
    fferr();
}


void f_makefs(void) {
    char xop=0;
    if(flags&FLAG_DIRECT) {
        unsigned char *f;
        f_data_t *e=get_data(stid,0);
        if(!e) {
            printf("The memory card will be formatted. Please supply a parameter \"Y\" to confirm\n");
            return;
        }
        e=get_text(0,&f);
        if(*f=='Y' || *f=='y') xop=1;
        set_data(stid,TYPE_MASK,0,0,-1);
    }
    if(xop) {
        FatFsErr=f_mkfs("",0,0);
        fferr();
    }
}


void f_dirf(void) {
    unsigned char deff[1]="";
    unsigned char *f=deff;
    f_data_t *e=get_data(stid,0);
    if(e) {
        e=get_text(0,&f); /* optional path */
        set_data(stid,TYPE_TEXT,0,f,(e->len+1));    /* increase the length of (e) by one character */
        e=get_text(0,&f);
        f[e->len-1]=0;
    }
    DIR dir;
    FatFsErr=f_opendir(&dir,f);
    if(FatFsErr!=FR_OK) {
        set_data(stid,TYPE_MASK,0,0,-1);
        fferr();
        return;
    }
    long p1=0;
    UINT s1,s2;
    s1=s2=0;
    new_line();
    for(;;) {
        FatFsErr=f_readdir(&dir,&Finfo);
        if(FatFsErr!=FR_OK || !Finfo.fname[0]) break;
        if(Finfo.fattrib & AM_DIR) {
            s2++;
        } else {
            s1++;
            p1+=Finfo.fsize;
        }
        printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %-12s  %s",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, Finfo.fsize, Finfo.fname,
#if _USE_LFN
                Lfname);
#else
                "");
#endif
        new_line();
    }
    printf("%4u file(s),  %10lu bytes total\n%4u dir(s)",s1,p1,s2);
    FATFS *fs=&FatFs;
    if(f_getfree(f,(DWORD*)&p1,&fs)==FR_OK) printf(",   %10lu bytes free\n",p1*fs->csize*_MAX_SS);
    set_data(stid,TYPE_MASK,0,0,-1);
    fferr();
}


void f_mkdirf(void) {
    unsigned char *f;
    f_data_t *e=get_text(0,&f); /* path */
    set_data(stid,TYPE_TEXT,0,f,(e->len+1));    /* increase the length of (e) by one character */
    e=get_text(0,&f);
    f[e->len-1]=0;
    FatFsErr=f_mkdir(f);
    set_data(stid,TYPE_MASK,0,0,-1);
    fferr();
}


void f_chdirf(void) {
    unsigned char *f;
    f_data_t *e=get_text(0,&f); /* path */
    set_data(stid,TYPE_TEXT,0,f,(e->len+1));    /* increase the length of (e) by one character */
    e=get_text(0,&f);
    f[e->len-1]=0;
    FatFsErr=f_chdir(f);
    set_data(stid,TYPE_MASK,0,0,-1);
    fferr();
}


void f_fcopyf(void) {
    unsigned char *from, *to;
    f_data_t *eo=get_text(1,&from);
    set_data(stid,TYPE_TEXT,0,from,(eo->len+1));
    eo=get_text(1,&from);
    f_data_t *en=get_text(0,&to);
    set_data(stid,TYPE_TEXT,0,to,(en->len+1));
    en=get_text(1,&to);
    FIL File[2];
    FatFsErr=f_open(&File[0], from, FA_OPEN_EXISTING | FA_READ);
    if(FatFsErr!=FR_OK) {
        fferr();
        return;
    }
    FatFsErr=f_open(&File[1], to, FA_CREATE_ALWAYS | FA_WRITE);
    if(FatFsErr!=FR_OK) {
        f_close(&File[0]);
        fferr();
        return;
    }
    BYTE Buff[256];
    long p1=0;
    UINT s1, s2;
    for (;;) {
        FatFsErr=f_read(&File[0], Buff, sizeof Buff, &s1);
        if (FatFsErr!=FR_OK || !s1) break;
        FatFsErr=f_write(&File[1], Buff, s1, &s2);
        if (FatFsErr!=FR_OK || s2<s1) break;
        p1+=s2;
    }
    FatFsErr=f_close(&File[0]);
    FatFsErr=f_close(&File[1]);
    fferr();
}


void f_fdelf(void) {
    unsigned char *s;
    f_data_t *e=get_text(0,&s);     /* filename */
    set_data(stid,TYPE_TEXT,0,s,(e->len+1));        /* increase the length of (e) by one character */
    e=get_text(0,&s);               /* reload (e) because its address might have changed */
    s[e->len-1]=0;                  /* assert a properly terminated string for passing to the C function */
    FatFsErr=f_unlink((TCHAR *)s);  /* perform the operation and return success flag */
    set_data(stid,TYPE_MASK,0,0,-1);
    fferr();
}


void f_frenf(void) {
    unsigned char *so, *sn;
    f_data_t *en=get_text(0,&sn);   /* new filename */
    set_data(stid,TYPE_TEXT,0,sn,(en->len+1));  /* increase the length of (eo) by one character */
    f_data_t *eo=get_text(1,&so);   /* old filename */
    set_data(stid,TYPE_TEXT,1,so,(eo->len+1));  /* increase the length of (eo) by one character */
    en=get_text(0,&sn);             /* reload (en) because its address might have changed */
    sn[en->len-1]=0;                /* assert a properly terminated string for passing to the C function */
    eo=get_text(1,&so);             /* reload (eo) because its address might have changed */
    so[eo->len-1]=0;                /* assert a properly terminated string for passing to the C function */
    FatFsErr=f_rename((TCHAR *)so,(TCHAR *)sn); /* perform the operation and return success flag */
    set_data(stid,TYPE_MASK,0,0,-1);
    set_data(stid,TYPE_MASK,0,0,-1);
    fferr();
}

/* overwritten atomic file functions */

void f_openf(void) {
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]);    /* search for free file handler */
    if(fh<0) longjmp(shell,ERROR_MANY_FILES);
    unsigned char *s;
    f_data_t *e=get_text(0,&s);     /* filename */
    set_data(stid,TYPE_TEXT,0,s,(e->len+1));        /* increase the length of (e) by one character */
    e=get_text(0,&s);               /* reload (e) because its address might have changed */
    s[e->len-1]=0;                  /* assert a properly terminated string for passing to the C function */
    FIL *f=&File[fh];
    FatFsErr=f_open(f,(char *)s,(FA_READ | FA_WRITE | FA_OPEN_ALWAYS));
    long l=-1;
    if(FatFsErr==FR_OK) {
        l=f_size(f);
        FatFsErr=f_lseek(f,0);
        if(FatFsErr==FR_OK) files[fh]=f; else l=-1; /* assign this file handler */
    }
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the input parameter */
    if(f) set_num(stid,TYPE_SINT,-1,(REAL)((unsigned long)f)); else set_num(stid,TYPE_SINT,-1,-1.0);    /* insert the file handler (returning the numeric value of the pointer to be used as a unique token) */
    set_num(stid,TYPE_SINT,-1,(REAL)l); /* insert the file length */
    fferr();
}


void f_closef(void) {
    long fh=MAX_FILES;
    FIL *ftkn=(FIL *)((unsigned long)get_num(0));
    if(ftkn) {
        while(--fh>=0 && files[fh]!=ftkn);
        if(fh<0) longjmp(shell,ERROR_UNKNOWN_FILE);
        FatFsErr=f_close(ftkn);
        files[fh]=0;    /* release this file handler */
    } else {
        while(--fh>=0)  {
            if(files[fh]) {
                f_close(files[fh]);
                files[fh]=0;
            }
        }
        FatFsErr=FR_OK;
    }
    fferr();
}


void f_eoff(void) {
    FIL *ftkn=(FIL *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    set_num(stid,TYPE_SINT,-1,(REAL)(!!f_eof(ftkn)));
}


void f_seekf(void) {
    unsigned long pos=(unsigned long)get_num(0);
    FIL *ftkn=(FIL *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    FatFsErr=f_lseek(ftkn,pos);
    if(FatFsErr==FR_OK) set_num(stid,TYPE_SINT,-1,(REAL)f_tell(ftkn)); else set_num(stid,TYPE_SINT,-1,-1.0);
    fferr();
}


void f_posf(void) {
    FIL *ftkn=(FIL *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    set_num(stid,TYPE_SINT,-1,(REAL)f_tell(ftkn));
}


void f_fout(void) {
    FIL *ftkn=(FIL *)((unsigned long)get_num(1));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    unsigned char *c=((unsigned char *)e+HDR);
    unsigned long l=0;
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) {
        FatFsErr=f_write(ftkn,(unsigned char *)c,e->len,(UINT *)&l);
    } else {
        REAL v=rrec_dbl(e);
        char buf[25] = {0};
        sprintf(buf,"%.10g",(double)v);
        FatFsErr=f_write(ftkn,(unsigned char *)buf,strlen(buf),(UINT *)&l);
    }
    set_num(stid,TYPE_UINT,0,(REAL)l);  /* convert the top element (which was the input data) into number of written characters output value */
    fferr();
}


void f_fin(void) {
    unsigned long r, l=get_num(0);
    if((pData-pCode)<=(2*HDR)) longjmp(shell,ERROR_NO_MEMORY);
    FIL *ftkn=(FIL *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    char *buf=(char *)(pCode+64);
    set_data(stid,TYPE_TEXT,-1,buf,l);
    FatFsErr=f_read(ftkn,(unsigned char *)buf,l,(UINT *)&r);
    set_data(stid,TYPE_TEXT,0,buf,r);   /* adjust the length according to the number of actually read characters */
    fferr();
}

/* end of file functions */
/* ========================================================================================================================= */


void f_option(void) {
    unsigned char *f;
    f_data_t *z=get_text(1,&f);     /* option id */
    f_data_t *e=get_data(stid,0);   /* option value */
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    unsigned char *c=((unsigned char *)e+HDR);  /* pointer to the raw data */
    if(!strncmp(f,"PageLines",9) && z->len==9) {
        int v=(int)rrec_dbl(e);
        if(v<0 || v>0x1000000000) longjmp(shell,ERROR_BAD_PARAMETER);
        sysvars.PageLines=v;
        lines_before_pause=v;
    }
    else if(!strncmp(f,"ConsoleBaudrate",15) && z->len==15) {
        int v=(int)rrec_dbl(e);
        if(v!=1200 && v!=2400 && v!=4800 && v!=9600 && v!=19200 && v!=38400 && v!=57600 && v!=115200) longjmp(shell,ERROR_BAD_PARAMETER);
        sysvars.ConsoleBaudrate=v;
        console_baudrate=v;
        console_init(); /* re-initialise the console with new baudrate */
    }
    else longjmp(shell,ERROR_INVALID_OPTION);
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the option value */
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the option id */
}


void f_u10tick(void) {
    unsigned int v=tick;
    set_num(stid,TYPE_UINT,-1,(REAL)v);
}


void f_pout(void) {
    int hw=(int)get_num(1);         /* device */
    f_data_t *e=get_data(stid,0);   /* data */
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    unsigned char *c=((unsigned char *)e+HDR);  /* pointer to the output data */
    unsigned long lc=0;                     /* data length counter */
    unsigned long len=e->len;
    char buf[25] = {0};
    if(((e->id)&TYPE_MASK)!=TYPE_TEXT) {    /* numeric data is first converted into text */
        REAL v=rrec_dbl(e);        
        sprintf(buf,"%.10g",(double)v);
        c=buf;
        len=strlen(buf);
    }
    while(lc<len) {
        if(hw==1) {         /* UART2 */
            if(!U2MODEbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);
            while(!U2STAbits.TRMT);
            U2TXREG=*(c++);
        }
        else if(hw==5) {    /* SPI1 */
            if(!SPI1CONbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);
            SPI1STATbits.SPIROV=0;
            while(!SPI1STATbits.SPITBE);
            SPI1BUF=*(c++);
        }
        else if(hw==8) {    /* I2C1 */
            if(!I2C1CONbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);

            // ### TODO


        }
        else longjmp(shell,ERROR_UNKNOWN_DEVICE);
        lc++;
    }
    set_num(stid,TYPE_UINT,0,(REAL)lc); /* convert the top element (which was the input data) into number of written characters output value */
}


void f_pin(void) {
    int hw=(int)get_num(1);     /* device */
    unsigned long l=get_num(0); /* readings */
    if((pData-pCode)<=(2*HDR)) longjmp(shell,ERROR_NO_MEMORY);    
    set_data(stid,TYPE_TEXT,-1,0,0);    /* create a new text element */
    f_data_t *e=get_data(stid,0);
    if(!e) if(!e) longjmp(shell,ERROR_NO_MEMORY);
    while(l--) {        
        unsigned char v=0;
        if(hw==1) {         /* UART2 */
            if(!U2MODEbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);
            if(u2_rx_in==u2_rx_out) break;  /* no available data */
            v=u2_rx[u2_rx_out++];
            if(u2_rx_out>=UART2_RX_BUFFER) u2_rx_out=0;
        }
        else if(hw==5) {    /* SPI1 */
            if(!SPI1CONbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);
            SPI1STATbits.SPIROV=0;
            while(!SPI1STATbits.SPITBE);
            SPI1BUF=0;
            while(!SPI1STATbits.SPIRBF);
            v=SPI1BUF;
        }
        else if(hw==8) {    /* I2C1 */
            if(!I2C1CONbits.ON) longjmp(shell,ERROR_NOT_INITIALISED);


            // ### TODO


        }
        else longjmp(shell,ERROR_UNKNOWN_DEVICE);
        set_data(stid,TYPE_TEXT,0,(unsigned char *)e+HDR,(e->len+1));   /* increase the length of (e) by one character */
        e=get_data(stid,0); /* reload (e) */
        if(!e) longjmp(shell,ERROR_NO_MEMORY);
        *((unsigned char *)e+HDR+e->len-1)=v;
    }
}


void f_pconfig(void) {
    int hw=(int)get_num(2);     /* device */
    unsigned char *f;
    f_data_t *e=get_text(1,&f); /* protocol */
    int b=(int)get_num(0);      /* baudrate */

    /* UART2 */
    if(hw==1) {
        if((b && b<200) || b>250000 || !f) longjmp(shell,ERROR_BAD_PARAMETER);
        /* if(U2MODEbits.ON && b && *f) longjmp(shell,ERROR_PORT_FUNC_ASSGN); */
        port[14].func&=~PFN_FUNC;
        port[15].func&=~PFN_FUNC;
        INTEnable(INT_SOURCE_UART_RX(UART2), INT_DISABLED);
        RPB10R=RPB11R=0;
        *(BASEB+TRISSET)=BIT(10);
        *(BASEB+CNPUCLR)=BIT(10);
        *(BASEA+TRISSET)=BIT(11);
        *(BASEB+CNPUCLR)=BIT(11);
        U2MODE=0;
        U2STA=0;
        if(b) {
            if(strncmp(f,"8N1",3) || e->len!=3) longjmp(shell,ERROR_BAD_PARAMETER); /* only 8N1 is supported for now */
            U2MODEbits.ON=1;    /* enable UART2 */
            U2MODEbits.SIDL=1;  /* stop in idle mode */
            U2MODEbits.BRGH=1;  /* use divider for high baudrates */            
            U2BRG=(currentCCLK/4/b)-1;  /* set baudrate */
            U2STAbits.URXEN=1;  /* enable RX */
            U2STAbits.UTXEN=1;  /* enable TX */
            *(BASEB+TRISSET)=BIT(11);
            *(BASEB+CNPUSET)=BIT(11);
            *(BASEB+TRISCLR)=BIT(10);
            *(BASEB+CNPUSET)=BIT(10);
            PPSInput(2,U2RX,RPB11);
            PPSOutput(4,RPB10,U2TX);
            port[14].func|=PFN_FUNC;
            port[15].func|=PFN_FUNC;
            if(U2MODEbits.ON) {
                INTEnable(INT_SOURCE_UART_RX(UART2), INT_ENABLED);
                INTSetVectorPriority(INT_VECTOR_UART(UART2), INT_PRIORITY_LEVEL_3);
            }
        }        
    }

    /* SPI1 */
    else if(hw==5) {
        int m=(int)rrec_dbl(e);
        if((b && b<50000) || b>20000000 || m<0 || m>3 || currentCCLK<(b*2)) longjmp(shell,ERROR_BAD_PARAMETER);
        /* if(SPI1CONbits.ON && b && m>=0) longjmp(shell,ERROR_PORT_FUNC_ASSGN); */
        if(b && m>=0 && (mkflags&MKFLG_MOUNTED)) longjmp(shell,ERROR_PORT_FUNC_ASSGN);  /* SPI port can't be released with currently mounted volume */
        port[0].func&=~PFN_FUNC;
        port[1].func&=~PFN_FUNC;
        port[2].func&=~PFN_FUNC;
        RPA1R=RPB5R=RPB14R=0;
        *(BASEA+TRISSET)=BIT(1);
        *(BASEB+TRISSET)=BIT(5);
        *(BASEA+TRISSET)=BIT(14);
        SPI1CON=0;
        SPI1CON2=0;
        if(b) {
            SPI1CONbits.ON=1;       /* enable SPI1 */
            SPI1CONbits.SIDL=1;     /* stop in idle mode */
            SPI1BRG=(currentCCLK/2/b)-1;
            if(m==0) SPI1CONbits.CKE=1;
            else if(m==3) SPI1CONbits.CKP=1;
            else if(m==2) {
                SPI1CONbits.CKE=1;
                SPI1CONbits.CKP=1;
            }
            SPI1CONbits.MSTEN=1;    /* enable master mode */
            SPI1CONbits.SRXISEL=3;
            SPI1STATbits.SPIROV=0;  /* clear the overrun flag */
            *(BASEB+TRISSET)=BIT(5);
            *(BASEA+TRISCLR)=BIT(1);
            *(BASEB+TRISCLR)=BIT(14);
            PPSInput(2,SDI1,RPB5);
            PPSOutput(2,RPA1,SDO1);
            port[0].func|=PFN_FUNC;
            port[1].func|=PFN_FUNC;
            port[2].func|=PFN_FUNC;
        }
    }

    /* I2C1 */
    else if(hw==8) {
        int m=(int)rrec_dbl(e);
        if((b && b<5000) || b>500000 || m<0 || m>1 || currentCCLK<(b*8)) longjmp(shell,ERROR_BAD_PARAMETER);
        port[11].func&=~PFN_FUNC;
        port[12].func&=~PFN_FUNC;
        RPB8R=RPB9R=0;
        *(BASEB+TRISSET)=BIT(8);
        *(BASEB+CNPUCLR)=BIT(8);
        *(BASEB+TRISSET)=BIT(9);
        *(BASEB+CNPUCLR)=BIT(9);
        I2C1CON=0;
        I2C1STAT=0;
        I2C1ADD=0;
        I2C1MSK=0;
        if(b) {
            I2C1CONbits.ON=1;               /* enable I2C1 */
            I2C1CONbits.SIDL=1;             /* stop in idle mode */
            if(m==1) I2C1CONbits.A10M=1;    /* 10-bit addressing (when specified) */
            I2C1BRG=(currentCCLK/(2*b))-2;  /* calculate the baudrate register value */
            if(I2C1BRG<2) I2C1BRG=2;
            *(BASEB+CNPUSET)=BIT(8);
            *(BASEB+CNPUSET)=BIT(9);
            port[11].func|=PFN_FUNC;
            port[12].func|=PFN_FUNC;
        }
    }

    else longjmp(shell,ERROR_UNKNOWN_DEVICE);
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the text parameter */
}


void f_cpuclk(void) {
    const unsigned int ir[8] = {150, 600, 1200, 2400, 4800, 4800, 9600, 9600};  /* interrupt rates according to CPU frequencies */
    int fq=(int)get_num(0);
    if(fq!=1 && fq!=4 && fq!=8 && fq!=12 && fq!=24 && fq!=32 && fq!=40 && fq!=48) longjmp(shell,ERROR_BAD_PARAMETER);
    INTDisableInterrupts();
    /* TODO ###
    currentCCLK=((unsigned long)fq*1000000);
    intrate=ir[fq];
    SYSTEMConfigPerformance(currentCCLK);
    */
    mOSCSetPBDIV(OSC_PB_DIV_1);
    console_init();
    timer_init();
    adc_init();
    INTEnableInterrupts();
}
