#include <stdio.h>
#include "platform.h"
#include "..\generic\fanf.h"


/*
store the library into file
NOTE: the name of this function must not be changed
*/
void _fanf_store(void) {
    FILE *f=fopen("words.fnf","w+b");
    if(f) {
        fanf_mpf(pCode,MPF_CLEANUP);
        fwrite(Fmem,1,pCode-Fmem,f);
        fclose(f);
    }
}


/*
restore the library from file
NOTE: the name of this function must not be changed
*/
void _fanf_restore(void) {
    FILE *f=fopen("words.fnf","r+b");
    if(f) {
        fseek(f,0,SEEK_END);
        unsigned long l=ftell(f);
        if(l) {
            rewind (f);
            unsigned long r=fread(Fmem,1,l,f);
            if(r) {
                pCode=&Fmem[r];
                pData=&Fmem[FMEM_SIZE];
                flags=0;
                *ns=0;
                fanf_mpf(pCode,MPF_CLEANUP);
            }
        }
        fclose(f);
    }
}
