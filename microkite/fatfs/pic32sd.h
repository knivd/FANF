#ifndef _PIC32_SD
#define _PIC32_SD

#include "ff.h"

/*-----------------------------------------------------------------------*/
/* Device Timer Driven Procedure                                         */
/*-----------------------------------------------------------------------*/
/* This function must be called by timer interrupt in period of 1ms      */
void disk_timerproc(void);

#endif
