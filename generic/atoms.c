#include <conio.h>  /* WARNING! this library may not exist in all platforms; used only for the kbhit() function */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "atoms.h"
#include "memory.h"
#include "fanf.h"
#include "shellw.h"

#ifdef MORE_WORDS_FILE
#include MORE_WORDS_FILE_INCLUDE(MORE_WORDS_FILE)
#endif

/* define the values of pi and e */
#define PI (REAL)(3.14159265358979323846)
#define E  (REAL)(2.71828182845904523536)


/*
helper functions, whichever of these is needed, it must be provided from outside and written for the particular platform
*/
extern void __attribute__ ((weak)) _fanf_store(void);   /* store the library into NVM */
extern void __attribute__ ((weak)) _fanf_restore(void); /* restore the library from NVM */
extern void __attribute__ ((weak)) _f_sysstr(char *s);  /* pass a NULL-terminated string (s) to the operating system for execution */


/*
read a number from a data record (from the data memory)
input
(*e) data record
output
rrec_dbl() the number read
*/
REAL rrec_dbl(f_data_t *e) {
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) longjmp(shell,ERROR_TYPE_MISMATCH);
    unsigned char *p=((unsigned char *)e+HDR);
    REAL v=0.0;
    if((e->id&TYPE_MASK)==TYPE_REAL) {  /* floating point numbers */
        REAL vr;
        memcpy(&vr,p,sizeof(REAL));
        v=(REAL)vr;
    }
    else if((e->len)==1) {  /* 8-bit integers */
        if(((e->id)&TYPE_MASK)==TYPE_SINT) {
            char vi=*((char *)p);
            v=(REAL)vi;
        } else {
            unsigned char vi=*((unsigned char *)p);
            v=(REAL)vi;
        }
    }
    else if((e->len)==2) {  /* 16-bit integers */
        if(((e->id)&TYPE_MASK)==TYPE_SINT) {
            short vi;
            memcpy(&vi,p,2);
            v=(REAL)vi;
        } else {
            unsigned short vi;
            memcpy(&vi,p,2);
            v=(REAL)vi;
        }
    }
    else if((e->len)==4) {  /* 32-bit integers */
        if(((e->id)&TYPE_MASK)==TYPE_SINT) {
            long vi;
            memcpy(&vi,p,4);
            v=(REAL)vi;
        } else {
            unsigned long vi;
            memcpy(&vi,p,4);
            v=(REAL)vi;
        }
    }
    return v;
}


/*
get the top number from the stack; will generate an error if the element contains text data
NOTE: the function will remove the top element from the stack
input
(osc) 'occurence skip counter' passed directly to the get_data() function; selects which in line element to get as 0 defines the top
output
get_num(0) value from the stack
*/
REAL get_num(unsigned int osc) {
    f_data_t *e=get_data(stid,osc);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    REAL v=rrec_dbl(e);
    set_data(stid,TYPE_MASK,osc,0,-1);  /* remove the element from the stack */
    return v;
}


/*
automatically determine the best size of a number and push it into the stack
input
(id) stack record id
(type) data type (see TYPE_xxx constants)
(osc) 'occurence skip counter' passed directly to the set_data() function
(n) the value to be pushed
*/
void set_num(unsigned int id, char type, int osc, REAL n) {
    char rf=0;

    /* check is it integer */
    if(type!=TYPE_REAL) {       /* previously: if((double)n==floor((double)n)) { */

        /* 8-bit signed constant */
        if(n>=-128 && n<=127) {
            char ni=(char)n;
            set_data(id,TYPE_SINT,osc,&ni,sizeof(char));
        }

        /* 8-bit unsigned constant */
        else if(n>=0 && n<=255) {
            unsigned char ni=(unsigned char)n;
            set_data(id,TYPE_UINT,osc,&ni,sizeof(unsigned char));
        }

        /* 16-bit signed constant */
        else if(n>=-32768 && n<=32767) {
            short ni=(short)n;
            set_data(id,TYPE_SINT,osc,&ni,sizeof(short));
        }

        /* 16-bit unsigned constant */
        else if(n>=0 && n<=65535) {
            unsigned short ni=(unsigned short)n;
            set_data(id,TYPE_UINT,osc,&ni,sizeof(unsigned short));
        }

        /* 32-bit signed constant */
        else if(n>=((REAL)0-0x80000000) && n<=0x7fffffff) {
            long ni=(long)n;
            set_data(id,TYPE_SINT,osc,&ni,sizeof(long));
        }

        /* 32-bit unsigned constant */
        else if(n>=0 && n<=0xffffffff) {
            unsigned long ni=(unsigned long)n;
            set_data(id,TYPE_UINT,osc,&ni,sizeof(unsigned long));
        }

        /* otherwise present as a floating point number */
        else rf=1;

    } else
        rf=1;

    /* floating point number constant or large integer numbers */
    if(rf) {
        REAL nr=(REAL)n;
        set_data(id,TYPE_REAL,osc,&nr,sizeof(REAL));
    }

}


/*
get the text element from the stack; will generate an error if the element is not of text type
NOTE: does not remove the element from the stack
input
(osc) 'occurence skip counter' passed directly to the set_data() function
(**data) pointer to data; will be loaded by the function
output
get_text() data element from the stack
(**data) pointer to the data chunk of the element
*/
f_data_t *get_text(int osc, unsigned char **data) {
    *data=0;
    f_data_t *e=get_data(stid,osc);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    if(!(((e->id)&TYPE_MASK)==TYPE_TEXT)) longjmp(shell,ERROR_TYPE_MISMATCH);
    *data=((unsigned char *)e+HDR);
    return e;
}


void f_new(void) {
    newdc[process]++;
}


void f_eow(void) {
    if(!newdc[process]) PC[process]=0; else newdc[process]--;
}


void f_call(void){
    unsigned long vi;
    get_vln(PC[process]+1,&vi);
    if(vi>=FMEM_SIZE) longjmp(shell,ERROR_MEMORY_REF);
    if(++nested>=MAX_NESTED) longjmp(shell,ERROR_NESTING);
    unsigned int ons=refns[process];
    refns[process]=vi;
    unsigned char *oPC=PC[process];
    fanf_execute(&Fmem[vi],NEWPF_CURRENT);
    PC[process]=oPC;
    refns[process]=ons;
    if(nested) nested--;
}


void f_task(void) {
    unsigned long vi;
    get_vln(PC[process]+1,&vi);
    if(vi>=FMEM_SIZE) longjmp(shell,ERROR_MEMORY_REF);
    taskPC=&Fmem[vi];
}


void f_ref(void) {
    unsigned long vi;
    get_vln(PC[process]+1,&vi);
    if(vi>=FMEM_SIZE) longjmp(shell,ERROR_MEMORY_REF);
    refid[process]=vi;

    /* perform an implied data get operation */
    f_data_t *e=get_data(refid[process],refix[process]);
    if(!e) {
        if(refix[process]) longjmp(shell,ERROR_INVALID_INDEX);

        /* if not initialised try to execute the word first */
        if(++nested>=MAX_NESTED) longjmp(shell,ERROR_NESTING);
        unsigned int ons=refns[process];
        refns[process]=vi;
        unsigned char *oPC=PC[process];
        fanf_execute(&Fmem[vi],NEWPF_CURRENT);
        PC[process]=oPC;
        refns[process]=ons;
        if(nested) nested--;

        /* check again */
        e=get_data(refid[process],refix[process]);
        if(!e) longjmp(shell,ERROR_UNDEFINED_VAR);
    }

    /* get the data container into stack */
    if(((e->id)&TYPE_MASK)==TYPE_TEXT)
        set_data(stid,((e->id)&TYPE_MASK),-1,((unsigned char *)e+HDR),e->len);
    else
        set_num(stid,((e->id)&TYPE_MASK),-1,rrec_dbl(e));
}


void f_set(void) {
    if(!refid[process]) longjmp(shell,ERROR_NO_REFERENCE);
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the top element (supposedly the one returned by the 'ref' instruction) */
    f_data_t *e=get_data(stid,0);       /* get the parameter */
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    if(!get_data(refid[process],refix[process])) {
        if(refix[process]) longjmp(shell,ERROR_INVALID_INDEX); else longjmp(shell,ERROR_UNDEFINED_VAR);
    }
    set_data(refid[process],((e->id)&TYPE_MASK),refix[process],((unsigned char *)e+HDR),e->len);
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the parameter */
    refix[process]=0;
}


void f_sint8(void) {
    char vi=*(PC[process]+1);
    REAL v=(REAL)vi;
    set_num(stid,TYPE_SINT,-1,v);
}


void f_uint8(void) {
    unsigned char vi=*(PC[process]+1);
    REAL v=(REAL)vi;
    set_num(stid,TYPE_UINT,-1,v);
}


void f_sint16(void) {
    int vi=0;
    char c;
    for(c=0; c<2; c++) vi=(vi<<8)+*(PC[process]+1+c);
    if(vi>32767) vi=-(((vi&0xffff)^0xffff)+1);
    REAL v=(REAL)((short)vi);
    set_num(stid,TYPE_SINT,-1,v);
}


void f_uint16(void) {
    unsigned int vi=0;
    char c;
    for(c=0; c<2; c++) vi=(vi<<8)+*(PC[process]+1+c);
    REAL v=(REAL)((unsigned short)vi);
    set_num(stid,TYPE_UINT,-1,v);
}


void f_sint32(void) {
    long vi=0;
    char c;
    for(c=0; c<4; c++) vi=(vi<<8)+*(PC[process]+1+c);
    if(vi>=0x80000000) vi=-(((vi&0xffffffff)^0xffffffff)+1);
    REAL v=(REAL)((long)vi);
    set_num(stid,TYPE_SINT,-1,v);
}


void f_uint32(void) {
    unsigned long vi=0;
    char c;
    for(c=0; c<4; c++) vi=(vi<<8)+*(PC[process]+1+c);
    REAL v=(REAL)((unsigned long)vi);
    set_num(stid,TYPE_UINT,-1,v);
}


void f_real(void) {
    REAL v;
    memcpy(&v,PC[process]+1,sizeof(REAL));
    set_num(stid,TYPE_REAL,-1,v);
}


void f_text(void) {
    unsigned long vi;
    unsigned char *s=get_vln(PC[process]+1,&vi);
    set_data(stid,TYPE_TEXT,-1,s,vi);
    addPC=vi;
}


void f_skip(void) {
    unsigned long vi;
    get_vln(PC[process]+1,&vi);
    addPC=vi;
}


void f_index(void) {
    int index=(int)get_num(0);
    if(index<0) longjmp(shell,ERROR_INVALID_INDEX);
    refix[process]=index;
}


void f_vsize(void) {
    if(!refid[process]) longjmp(shell,ERROR_NO_REFERENCE);
    set_data(stid,TYPE_MASK,0,0,-1);    /* remove the top element (supposedly the one returned by the 'ref' instruction) */
    REAL v=0.0;
    while(get_data(refid[process],v)) v+=1.0;
    set_num(stid,TYPE_SINT,0,v);
}


void f_data(void) {
    if(!refns[process]) longjmp(shell,ERROR_NAMESPACE_OUT);
    int x=0;
    while (get_data(refns[process],x)) x++; /* count the number of currently assigned elements */
    int n=(int)get_num(0);  /* get the new size from stack */
    if(n==x) return;    /* no change */
    if(n<x) {   /* reducing size - deleting elements */
        while(x>n) {
            set_data(refns[process],TYPE_MASK,0,0,-1);
            x--;
        }
    } else {    /* increasing size - adding blank elements */
        while(x<n) {
            set_data(refns[process],TYPE_TEXT,-1,0,0);
            x++;
        }
    }
}


void f_if(void) {
    if(ctldc[process]>=MAX_CTRL) longjmp(shell,ERROR_NESTED_CTRL);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(ctldc[process] && (c->type==BLANK || (c->type==IFEND && !(c->condition)))) {
        ctrl[process][ctldc[process]].type=BLANK;
    } else {
        ctrl[process][ctldc[process]].type=IFEND;
        ctrl[process][ctldc[process]].condition=(long)get_num(0);
        ctrl[process][ctldc[process]].contf=0;
    }
    ctldc[process]++;
}


void f_else(void) {
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(!ctldc[process] || (c->type!=IFEND && c->type!=BLANK)) longjmp(shell,ERROR_UNEXP_ELSE);
    if(c->type!=BLANK) c->condition=!(c->condition);
}


void f_endif(void) {
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(!ctldc[process] || (c->type!=IFEND && c->type!=BLANK)) longjmp(shell,ERROR_UNEXP_ENDIF);
    ctldc[process]--;
}


void f_do(void) {
    if(ctldc[process]>=MAX_CTRL) longjmp(shell,ERROR_NESTED_CTRL);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(ctldc[process] && c->type==BLANK) {
        ctrl[process][ctldc[process]].type=BLANK;
    } else {
        ctrl[process][ctldc[process]].type=DOREP;
        ctrl[process][ctldc[process]].contf=0;
        ctrl[process][ctldc[process]].retaddr=PC[process];
    }
    ctldc[process]++;
}


void f_while(void) {
    if(ctldc[process]>=MAX_CTRL) longjmp(shell,ERROR_NESTED_CTRL);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(ctldc[process] && c->type==BLANK) {
        ctrl[process][ctldc[process]].type=BLANK;
    } else {
        if(!refid[process]) longjmp(shell,ERROR_NO_REFERENCE);
        if(get_num(0)) {    /* get the variable value returned by the preceding 'ref' instruction */
            ctrl[process][ctldc[process]].type=WHREP;
            ctrl[process][ctldc[process]].condition=refid[process];
            ctrl[process][ctldc[process]].index=refix[process];
            ctrl[process][ctldc[process]].contf=0;
            ctrl[process][ctldc[process]].retaddr=PC[process];
        } else {
            ctrl[process][ctldc[process]].type=BLANK;
        }
    }
    ctldc[process]++;
}


void f_loop(void) {
    if(ctldc[process]>=MAX_CTRL) longjmp(shell,ERROR_NESTED_CTRL);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(ctldc[process] && c->type==BLANK) {
        ctrl[process][ctldc[process]].type=BLANK;
    } else {
        ctrl[process][ctldc[process]].type=LPREP;
        ctrl[process][ctldc[process]].retaddr=PC[process];
        ctrl[process][ctldc[process]].condition=(long)get_num(0);
        ctrl[process][ctldc[process]].contf=0;
    }
    ctldc[process]++;
}


void f_repeat(void) {
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    if(!ctldc[process] || (c->type!=DOREP && c->type!=WHREP &&c->type!=LPREP && c->type!=BLANK)) longjmp(shell,ERROR_UNEXP_REPEAT);
    c->contf=0;
    if(c->type==DOREP) {        /* do...repeat */
        if(get_num(0)) PC[process]=c->retaddr; else ctldc[process]--;
    }
    else if(c->type==WHREP) {   /* while...repeat */
        f_data_t *e=get_data(c->condition,c->index);
        if(!e) {
            if(c->index) longjmp(shell,ERROR_INVALID_INDEX); else longjmp(shell,ERROR_UNDEFINED_VAR);
        }
        if(rrec_dbl(e)) PC[process]=c->retaddr; else ctldc[process]--;
    }
    else if(c->type==LPREP) {   /* loop...repeat */
        if(c->condition) {
            if(--(c->condition)) PC[process]=c->retaddr; else ctldc[process]--;
        } else {
            PC[process]=c->retaddr; /* 0 loop ... repeat */
        }                           /* is an endless loop */
    } else {    /* void */
        ctldc[process]--;
    }
}


void f_cont(void) {
    if(!ctldc[process]) longjmp(shell,ERROR_UNEXP_CONT);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    c->contf=1;
}


void f_break(void) {
    if(!ctldc[process]) longjmp(shell,ERROR_UNEXP_BREAK);
    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
    c->type=BLANK;
}


void f_end(void) {
    PC[process]=0;
}


void f_endall(void) {
    memset(PC,0,sizeof(PC));
}


void f_clear(void) {
    f_data_t *e=(f_data_t *)pData;
    do {    /* delete all stack elements for the current process */
        e=get_data(stid,0);
        if(e && e->id!=SOURCE) set_data(stid,TYPE_MASK,0,0,-1);
    } while(e);
}


void f_empty(void) {
    f_data_t *e=(f_data_t *)pData;
    do {    /* delete all stack elements for the current process */
        e=get_data(stid,0);
        if(e) set_data(stid,TYPE_MASK,0,0,-1);
    } while(e);
}


void f_depth(void) {
    REAL v=0.0;
    f_data_t *e=(f_data_t *)pData;
    do {    /* count all stack elements for the current process */
        e=get_data(stid,v);
        if(e) {
            v+=1.0;
            e=(f_data_t *)(((unsigned char *)e+HDR)+e->len);
        }
    } while(e);
    set_num(stid,TYPE_SINT,-1,v);
}


void f_drop(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    set_data(stid,TYPE_MASK,0,0,-1);
}


void f_dup(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    set_data(e->id,(e->id&TYPE_MASK),-1,((unsigned char *)e+HDR),e->len);
}


void f_copy(void) {
    int x=(int)get_num(0);
    if(x<0) return;
    f_data_t *e=get_data(stid,x);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    set_data(e->id,(e->id&TYPE_MASK),-1,((unsigned char *)e+HDR),e->len);
}


void f_swap(void) {
    int x=(int)get_num(0);
    if(x<1) return;
    f_data_t *i=get_data(stid,x);   /* get the x-th internal element */
    if(!i) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *t=get_data(stid,0);   /*  get the top element */
    if(!t) longjmp(shell,ERROR_STACK_EMPTY);
    set_data(i->id,(i->id&TYPE_MASK),-1,(unsigned char *)i+HDR,i->len);     /* add the internal element on top */
    i->id=t->id;
    set_data(i->id,(i->id&TYPE_MASK),x+1,(unsigned char *)t+HDR,t->len);    /* update the internal element */
    set_data(stid,TYPE_MASK,1,0,-1);    /* remove the old top (ie. currently second in line) */
}


void f_type(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    REAL v=0.0;
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) v=1;
    set_num(stid,TYPE_SINT,0,v);
}


void f_astext(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) return;
    REAL v=rrec_dbl(e);
    char buf[25] = {0};
    sprintf(buf,"%.10g",(double)v);
    set_data(stid,TYPE_TEXT,0,buf,strlen(buf));
}


void f_asnum(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    if(((e->id)&TYPE_MASK)!=TYPE_TEXT) return;
    REAL v=0.0;
    if(e->len<25) {
        char buf[25] = {0};
        memcpy(buf,((unsigned char *)e+HDR),e->len);
        v=strtod((const char *)buf,NULL);
    }
    set_num(stid,TYPE_REAL,0,v);
}


void f_isnum(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    REAL v;
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) {
        if(e->len && e->len<25) {
            char *xp;
            char buf[25] = {0};
            memcpy(buf,((unsigned char *)e+HDR),e->len);
            v=strtod((const char *)buf,&xp);
            if((xp-buf)==e->len) v=1.0; else v=0.0;
        } else {
            v=0.0;
        }
    } else {
        v=1.0;
    }
    set_num(stid,TYPE_SINT,0,v);
}


void f_m8w(void) {
	volatile unsigned char *m=(unsigned char *)((unsigned long)get_num(0));     /* address */
	*m=(unsigned char)get_num(0);   /* value */
}


void f_m8r(void) {
    volatile unsigned char *m=(unsigned char *)((unsigned long)get_num(0));     /* address */
    REAL d=(REAL)*m;
    set_num(stid,TYPE_UINT,-1,d);
}


void f_m16w(void) {
    volatile unsigned short *m=(unsigned short *)((unsigned long)get_num(0));   /* address */
	*m=(unsigned short)get_num(0);  /* value */
}


void f_m16r(void) {
    volatile unsigned short *m=(unsigned short *)((unsigned long)get_num(0));   /* address */
    REAL d=(REAL)*m;
    set_num(stid,TYPE_UINT,-1,d);
}


void f_m32w(void) {
    volatile unsigned long *m=(unsigned long *)((unsigned long)get_num(0));     /* address */
	*m=(unsigned long)get_num(0);   /* value */
}


void f_m32r(void) {
    volatile unsigned long *m=(unsigned long *)((unsigned long)get_num(0));     /* address */
    REAL d=(REAL)*m;
    set_num(stid,TYPE_UINT,-1,d);
}


void f_not(void) {
    REAL d=get_num(0);
    d=(REAL)(~(unsigned long)d);
    set_num(stid,TYPE_UINT,-1,d);
}


void f_and(void) {
    REAL d=(REAL)(((unsigned long)get_num(0))&((unsigned long)get_num(0)));
    set_num(stid,TYPE_UINT,-1,d);
}


void f_or(void) {
    REAL d=(REAL)(((unsigned long)get_num(0))|((unsigned long)get_num(0)));
    set_num(stid,TYPE_UINT,-1,d);
}


void f_xor(void) {
    REAL d=(REAL)(((unsigned long)get_num(0))^((unsigned long)get_num(0)));
    set_num(stid,TYPE_UINT,-1,d);
}


void f_shl(void) {
    REAL d=get_num(0);
    d=(REAL)(((unsigned long)get_num(0))<<((unsigned char)d));
    set_num(stid,TYPE_UINT,-1,d);
}


void f_shr(void) {
    REAL d=get_num(0);
    d=(REAL)(((unsigned long)get_num(0))>>((unsigned char)d));
    set_num(stid,TYPE_UINT,-1,d);
}


void f_is(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    set_num(stid,TYPE_SINT,-1,(REAL)!!e->len);
}


void f_teq(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=0.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT) && x->len==y->len && !memcmp((unsigned char *)x+HDR,(unsigned char *)y+HDR,x->len)) {
        d=1.0;
    } else {
        if(((xt&TYPE_MASK)!=TYPE_TEXT) && ((yt&TYPE_MASK)!=TYPE_TEXT)) {
            REAL xd=rrec_dbl(x);
            REAL yd=rrec_dbl(y);
            if(xd==yd) d=1.0;
        }
    }
    set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_tne(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=1.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT) && x->len==y->len && !memcmp((unsigned char *)x+HDR,(unsigned char *)y+HDR,x->len)) {
        d=0.0;
    } else {
        if(((xt&TYPE_MASK)!=TYPE_TEXT) && ((yt&TYPE_MASK)!=TYPE_TEXT)) {
            REAL xd=rrec_dbl(x);
            REAL yd=rrec_dbl(y);
            if(xd==yd) d=0.0;
        }
    }
	set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_tgr(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=0.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT)) {
        if(x->len>y->len) d=1.0;
        else if(x->len==y->len) {
            unsigned char *xs=(unsigned char *)x+HDR;
            unsigned char *ys=(unsigned char *)y+HDR;
            unsigned int l=x->len;
            while(l-- && d==0.0) {
                if(*(xs++)>*(ys++)) d=1.0;
            }
        }
    }
    else {
        if(((xt&TYPE_MASK)==TYPE_TEXT) || ((yt&TYPE_MASK)==TYPE_TEXT)) longjmp(shell,ERROR_TYPE_MISMATCH);
        REAL xd=rrec_dbl(x);
        REAL yd=rrec_dbl(y);
        if(xd>yd) d=1.0;
    }
	set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_tge(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=0.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT)) {
        if(x->len>=y->len) d=1.0;
        else if(x->len==y->len) {
            unsigned char *xs=(unsigned char *)x+HDR;
            unsigned char *ys=(unsigned char *)y+HDR;
            unsigned int l=x->len;
            while(l-- && d==0.0) {
                if(*(xs++)>=*(ys++)) d=1.0;
            }
        }
    }
    else {
        if(((xt&TYPE_MASK)==TYPE_TEXT) || ((yt&TYPE_MASK)==TYPE_TEXT)) longjmp(shell,ERROR_TYPE_MISMATCH);
        REAL xd=rrec_dbl(x);
        REAL yd=rrec_dbl(y);
        if(xd>=yd) d=1.0;
    }
	set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_tsm(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=0.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT)) {
        if(x->len<y->len) d=1.0;
        else if(x->len==y->len) {
            unsigned char *xs=(unsigned char *)x+HDR;
            unsigned char *ys=(unsigned char *)y+HDR;
            unsigned int l=x->len;
            while(l-- && d==0.0) {
                if(*(xs++)<*(ys++)) d=1.0;
            }
        }
    }
    else {
        if(((xt&TYPE_MASK)==TYPE_TEXT) || ((yt&TYPE_MASK)==TYPE_TEXT)) longjmp(shell,ERROR_TYPE_MISMATCH);
        REAL xd=rrec_dbl(x);
        REAL yd=rrec_dbl(y);
        if(xd<yd) d=1.0;
    }
	set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_tse(void) {
    f_data_t *x=get_data(stid,1);
    if(!x) longjmp(shell,ERROR_STACK_EMPTY);
    f_data_t *y=get_data(stid,0);
    if(!y) longjmp(shell,ERROR_STACK_EMPTY);
    char xt=(x->id)&TYPE_MASK;
    char yt=(y->id)&TYPE_MASK;
	REAL d=0.0;
    if(((xt&TYPE_MASK)==TYPE_TEXT) && ((yt&TYPE_MASK)==TYPE_TEXT)) {
        if(x->len<=y->len) d=1.0;
        else if(x->len==y->len) {
            unsigned char *xs=(unsigned char *)x+HDR;
            unsigned char *ys=(unsigned char *)y+HDR;
            unsigned int l=x->len;
            while(l-- && d==0.0) {
                if(*(xs++)<=*(ys++)) d=1.0;
            }
        }
    }
    else {
        if(((xt&TYPE_MASK)==TYPE_TEXT) || ((yt&TYPE_MASK)==TYPE_TEXT)) longjmp(shell,ERROR_TYPE_MISMATCH);
        REAL xd=rrec_dbl(x);
        REAL yd=rrec_dbl(y);
        if(xd<=yd) d=1.0;
    }
	set_data(stid,TYPE_MASK,0,0,-1);
	set_num(stid,TYPE_SINT,0,d);
}


void f_sign(void) {
    REAL d=get_num(0);
    if(d>=0.0) d=1.0; else d=-1.0;
    set_num(stid,TYPE_SINT,-1,d);
}


void f_add(void) {
    REAL d=get_num(0);
    d=get_num(0)+d;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_sub(void) {
    REAL d=get_num(0);
    d=get_num(0)-d;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_mul(void) {
    REAL d=get_num(0);
    d=get_num(0)*d;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_div(void) {
    REAL d=get_num(0);
    if(d==0.0) longjmp(shell,ERROR_DIV_ZERO);
    d=get_num(0)/d;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_mod(void) {
    REAL d=get_num(0);
    if(d==0.0) longjmp(shell,ERROR_DIV_ZERO);
    d=(REAL)(((long)get_num(0))%((long)d));
    set_num(stid,TYPE_SINT,-1,d);
}


void f_inc(void) {
    REAL d=get_num(0)+1.0;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_dec(void) {
    REAL d=get_num(0)-1.0;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_abs(void) {
    REAL d=abs(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_round(void) {
#if 0
    REAL d=round(get_num(0));
#else
    /* this is an alternative in case the round() function is missing in <math.h> */
    REAL d=get_num(0);
    if(d!=floor(d)) {
        REAL r=abs(d);
        if((r-floor(r))>=0.5) r=ceil(r); else r=floor(r);
        if(d>=0.0) d=r; else d=-r;
    }
#endif
    set_num(stid,TYPE_REAL,-1,d);
}


void f_trim(void) {
    REAL d=floor(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_power(void) {
    REAL d;
	REAL y=get_num(0);
	REAL x=get_num(0);
	if(((x==0.0) && (y<=0.0)) || ((x<0.0) && (y!=floor(y)))) longjmp(shell,ERROR_INVALID_MATH);
	if(y>=0.0) d=pow(x,y); else d=1/pow(x,abs(y));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_sin(void) {
    REAL d=sin(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_cos(void) {
    REAL d=cos(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_tan(void) {
    REAL d=tan(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_atan(void) {
    REAL d=atan(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_rad(void) {
    REAL d=get_num(0);
    d*=(PI/180.0);
    set_num(stid,TYPE_REAL,-1,d);
}


void f_deg(void) {
    REAL d=get_num(0);
    d*=(180.0/PI);
    set_num(stid,TYPE_REAL,-1,d);
}


void f_ln(void) {
    REAL d=log(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_exp(void) {
    REAL d=exp(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_log(void) {
    REAL d=log10(get_num(0));
    set_num(stid,TYPE_REAL,-1,d);
}


void f_PI(void) {
    REAL d=PI;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_E(void) {
    REAL d=E;
    set_num(stid,TYPE_REAL,-1,d);
}


void f_rnd(void) {
    REAL d=((REAL)rand())/((REAL)RAND_MAX);
    set_num(stid,TYPE_REAL,-1,d);
}


void f_rndseed(void) {
    srand((long)get_num(0));
}


void f_len(void) {
    f_data_t *e=get_data(stid,0);
    unsigned long l=e->len;
    set_num(stid,TYPE_UINT,0,l);
}


void f_concat(void) {
    unsigned char *ec,*rc;
    f_data_t *e=get_text(0,&ec);    /* data to be added */
    f_data_t *r=get_text(1,&rc);    /* original text */
    if(e->len) {
        unsigned int rl=r->len;
        set_data(stid,TYPE_TEXT,1,rc,(r->len+e->len));  /* change size of the original element (r) */
        e=get_text(0,&ec);
        r=get_text(1,&rc);
        memmove((unsigned char *)(rc+rl),ec,e->len);
        set_data(stid,TYPE_TEXT,1,rc,r->len);   /* update the top element (r) */
    }
    set_data(stid,TYPE_MASK,0,0,-1);  /* delete the top element from the stack (e) */
}


void f_cut(void) {
    int c=(int)get_num(0);  /* count */
    int b=(int)get_num(0);  /* beginning */
    unsigned char *s;
    f_data_t *e=get_text(0,&s);
    if((b+c)<=0) return;
    if(b<0) {
        c+=b;
        b=0;
    }
    if(c<0) c=(e->len)-b;
    if((b+c)>e->len) c=e->len-b;
    if(c) memmove(s,s+b,c);
    set_data(stid,TYPE_TEXT,0,s,c);
}


void f_delete(void) {
    int c=(int)get_num(0);  /* count */
    int b=(int)get_num(0);  /* beginning */
    unsigned char *s;
    f_data_t *e=get_text(0,&s);
    if((b+c)<=0 || !c) return;
    if(b<0) {
        c+=b;
        b=0;
    }
    if(c<0) c=(e->len)-b;
    if((b+c)>e->len) c=e->len-b;
    if(c) memmove(s+b,s+b+c,c);
    set_data(stid,TYPE_TEXT,0,s,(e->len-c));
}


void f_insert(void) {
    int b=(int)get_num(0);  /* beginning */
    unsigned char *ec,*rc;
    f_data_t *e=get_text(0,&ec);    /* data to be added */
    f_data_t *r=get_text(1,&rc);    /* original text */
    unsigned int rl=r->len;
    if(b<0) b=0;
    if(b>rl) b=rl;
    set_data(stid,TYPE_TEXT,1,rc,(r->len+e->len));  /* change size of the original element (r) */
    e=get_text(0,&ec);
    r=get_text(1,&rc);
    memmove((unsigned char *)(rc+b+e->len),(unsigned char *)(rc+b),(rl-b));
    memmove((unsigned char *)(rc+b),ec,e->len);
    set_data(stid,TYPE_TEXT,1,rc,r->len);   /* update the top element (r) */
    set_data(stid,TYPE_MASK,0,0,-1);        /* delete the top element from the stack (e) */
}


void f_replace(void) {
    int b=(int)get_num(0);  /* beginning */
    unsigned char *ec,*rc;
    f_data_t *e=get_text(0,&ec);    /* data to be added */
    f_data_t *r=get_text(1,&rc);    /* original text */
    if(b<0) b=0;
    if(b>r->len) b=r->len;
    if((b+e->len)>r->len) set_data(stid,TYPE_TEXT,1,rc,(b+e->len)); /* modify size of the original element (r) if needed */
    e=get_text(0,&ec);
    r=get_text(1,&rc);
    memmove((unsigned char *)(rc+b),ec,e->len);
    set_data(stid,TYPE_TEXT,1,rc,r->len);   /* update the top element (r) */
    set_data(stid,TYPE_MASK,0,0,-1);        /* delete the top element from the stack (e) */
}


void f_scan(void) {
    int b=(int)get_num(0);  /* start index of the search */
    unsigned char *ec,*rc;
    f_data_t *e=get_text(0,&ec);    /* searched sequence */
    f_data_t *r=get_text(1,&rc);    /* original text */
    int x=-1;
    if(b>=0 && e->len && (b+e->len)<=r->len) {
        x=b;
        while((x+e->len)<=r->len && memcmp(rc+x,ec,e->len)) x++;
        if((x+e->len)>r->len) x=-1;
    }
    set_data(stid,TYPE_MASK,0,0,-1);
    set_num(stid,TYPE_SINT,0,(REAL)x);
}


void f_use(void) {
    unsigned char *s;
    f_data_t *e=get_text(0,&s);
    if((e->len+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
    char tn[NAMESPACE_SIZE] = {0};
    if(e->len) memcpy(tn,s,e->len);
    set_data(stid,TYPE_MASK,0,0,-1);
    if(*tn) {
        const f_atom_t *a=find_atom(tn,0);
        if(a && a->func) {
            (a->func)();
            return;
        }
        if(*tn && tn[strlen(tn)]!=':') strcat(tn,":");  /* assert a lexically valid namespace */
        const unsigned char *w=find_word(tn);
        if(!w) longjmp(shell,ERROR_UNKNOWN_WORD);
        if(++nested>=MAX_NESTED) longjmp(shell,ERROR_NESTING);
        unsigned char *oPC=PC[process];
        fanf_execute(w,NEWPF_CURRENT);
        PC[process]=oPC;
        if(nested) nested--;
    }
}


void f_isknown(void) {
    unsigned char *s;
    f_data_t *e=get_text(0,&s);
    if((e->len+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
    char tn[NAMESPACE_SIZE] = {0};
    if(e->len) memcpy(tn,s,e->len);
    unsigned char res=0;
    if(*tn) {
        const f_atom_t *a=find_atom(tn,0);
        if(!a) {
            if(*tn && tn[strlen(tn)]!=':') strcat(tn,":");  /* assert a lexically valid namespace */
            const unsigned char *w=find_word(tn);
            if(w) res=2;
        } else {
            res=1;
        }
    }
    set_num(stid,TYPE_SINT,0,(REAL)res);
}


/*
compile (and execute) FANF source code
*/
void f_source(void) {
    unsigned int id;
    if(!PC[process] || !stid) id=SOURCE; else id=stid;
    char *s;
    f_data_t *e=get_data(id,0);
    if(!e) return;
    set_data(id,(id&TYPE_MASK),0,(unsigned char *)e+HDR,(e->len+1));    /* increase the length of (e) by one character */
    e=get_data(id,0);   /* reload (e) */
    if(!e) longjmp(shell,ERROR_INTERNAL_ERROR);
    s=(char *)e+HDR;
    s[e->len-1]=0;  /* assert a properly terminated the input source */
    fsrc=s;
    while((fsrc-s)<e->len) {
        while(*fsrc<=' ' && (fsrc-s)<e->len) fsrc++;    /* skip leading junk characters */
        unsigned char *opCode=pCode;
        flags&=~FLAG_WAS_EOW;
        flags|=FLAG_COMPILE;
        if((fsrc-s)<e->len) fanf_compile_word();
        flags&=~(FLAG_COMPILE | FLAG_KEEP_PCODE | FLAG_DIRECT);
        if(!(*ns) && !(flags&FLAG_WAS_EOW) && !(flags&FLAG_FORCE_CMPL)) {   /* words in direct mode are executed immediately and not stored into the code memory */
            flags|=FLAG_DIRECT;
            if(!PC[process]) {
                fanf_execute(opCode,NEWPF_NEW);
            } else {
                if(++nested>=MAX_NESTED) longjmp(shell,ERROR_NESTING);
                unsigned int ons=refns[process];
                refns[process]=(unsigned long)(opCode-Fmem);
                unsigned char *oPC=PC[process];
                fanf_execute(opCode,NEWPF_CURRENT);
                PC[process]=oPC;
                refns[process]=ons;
                if(nested) nested--;
            }
            if(!(flags&FLAG_KEEP_PCODE)) pCode=opCode;
            e=get_data(id,0);   /* reload (e) again because its position in the stack might have been changed recently */
            if(!e) break;   /* previously: longjmp(shell,ERROR_INTERNAL_ERROR); */
            char *next=((char *)e+HDR);
            fsrc+=(next-s);
            s=next;
        }
    }
    set_data(id,TYPE_MASK,0,0,-1);
    if(pCode>Fmem && *(pCode-1)!=code[CODE_NOP]) add_atom("%nop");  /* mark the end of this source line */
}


void f_forget(void) {
    unsigned char *s;
    f_data_t *e=get_text(0,&s);
    if((e->len+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
    char tn[NAMESPACE_SIZE] = {0};
    if(e->len) memcpy(tn,s,e->len);
    set_data(stid,TYPE_MASK,0,0,-1);  /* remove the parameter from the stack */
    if(*tn) {
        if(*tn && tn[strlen(tn)]!=':') strcat(tn,":");  /* assert a lexically valid namespace */
        const unsigned char *w=find_word(tn);
        if(!w) longjmp(shell,ERROR_UNKNOWN_WORD);
        unsigned int cid=(unsigned int)(w-Fmem)&(~TYPE_MASK);
        unsigned char *p=pData;
        while(p<(Fmem+FMEM_SIZE)) { /* remove from stack all data containers associated with the removed words */
            if(((f_data_t *)p)->id<SOURCE && ((f_data_t *)p)->id>=cid) {
                set_data(((f_data_t *)p)->id,(((f_data_t *)p)->id&TYPE_MASK),0,0,-1);
                p=pData;
            } else {
                p+=(HDR+((f_data_t *)p)->len);
            }
        }
        pCode=(unsigned char *)w;
        int t;      /* immediately cancel all processes currently executing the removed words */
        for(t=0; t<MAX_PROCESS; t++) if(PC[t]>=pCode) PC[t]=0;
        flags|=FLAG_KEEP_PCODE; /* set this flag in order to prevent the saved [pCode] be restored over the new one */
    }
    fanf_mpf(pCode,MPF_CLEANUP);
}


void f_threads(void) {
    REAL v=0.0;
    int p;
    for(p=0; p<MAX_PROCESS; p++) if(PC[p]) v+=1.0;
    set_num(stid,TYPE_SINT,-1,v);
}


void f_maxthreads(void) {
    REAL v=MAX_PROCESS;
    set_num(stid,TYPE_SINT,-1,v);
}


void f_free(void) {
    REAL v=(REAL)(pData-pCode);
    set_num(stid,TYPE_UINT,-1,v);
}


void f_machine(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    void (*machcode)()=(void *)(((unsigned char *)e+HDR));
    machcode();
    set_data(stid,TYPE_MASK,0,0,-1);
}


void f_cout(void) {
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    unsigned char *c=((unsigned char *)e+HDR);
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) {
        unsigned int l=e->len;
        while(l--) printf("%c",*(c++));
    } else {
        REAL v=rrec_dbl(e);
        printf("%.10g",(double)v);
    }
    set_data(stid,TYPE_MASK,0,0,-1);
}


void f_cin(void) {
    char *buf=(char *)(pCode+64);
    while(kbhit()) getchar();
    line_edit(buf,(unsigned long)(pData-pCode)/2-2*HDR,0);
    set_data(stid,TYPE_TEXT,-1,buf,strlen(buf));
}


void f_nwcin(void) {
    int l=(int)get_num(0);  /* number of readings */
    if((pData-pCode)<=(2*HDR)) longjmp(shell,ERROR_NO_MEMORY);
    set_data(stid,TYPE_TEXT,-1,0,0);    /* create a new text element */
    f_data_t *e=get_data(stid,0);
    if(!e) if(!e) longjmp(shell,ERROR_NO_MEMORY);
    while(kbhit() && l--) {
        set_data(stid,TYPE_TEXT,0,(unsigned char *)e+HDR,(e->len+1));   /* increase the length of (e) by one character */
        e=get_data(stid,0); /* reload (e) */
        if(!e) longjmp(shell,ERROR_NO_MEMORY);
        *((unsigned char *)e+HDR+e->len-1)=getchar();
    }
}


/* modifier flags used in f_format() */
#define FMT_FLG_SPEC    BIT(0)
#define FMT_FLG_STORE   BIT(1)
#define FMT_FLG_LALGN   BIT(2)
#define FMT_FLG_RALGN   BIT(3)
#define FMT_FLG_CENTRE  BIT(4)
#define FMT_FLG_LP      BIT(5)
#define FMT_FLG_LZERO   BIT(6)
#define FMT_FLG_TZERO   BIT(7)
#define FMT_FLG_PLUS    BIT(8)
#define FMT_FLG_RSVSGN  BIT(9)
#define FMT_TYP_TEXT    0
#define FMT_TYP_DEC     1
#define FMT_TYP_HEX     2
#define FMT_TYP_BIN     3

void f_format(void) {
    unsigned char *vs, *f;
    set_data(stid,TYPE_TEXT,-1,0,0);    /* create a new data element (output) */
    f_data_t *v=get_text(0,&vs);        /* get the output */
    f_data_t *e=get_text(1,&f);         /* get the format string */
    int stx=1;      /* stack index for the incoming parameters */
    int x=0;
    while(x<(e->len-1)) {   /* count how many parameters should be taken from the stack */
        if(f[x]=='|' && f[++x]!='|') stx++;
        x++;
    }
    int param=stx;
    char fillchr=' ';   /* fill character for text */
    char ftype=-1;
    unsigned int fflag=0;
    int len=0, frlen=0;
    x=0;
    while(x<=e->len) {
        if(x<e->len && f[x]=='|') {
            if(++x>=e->len) longjmp(shell,ERROR_INVALID_FMT);
            if(f[x]!='|') fflag|=FMT_FLG_SPEC;
        }
        if(fflag&FMT_FLG_SPEC) {
            if(x<e->len && f[x]=='#') {         /* decimal numbers */
                if(ftype<0) ftype=FMT_TYP_DEC; else longjmp(shell,ERROR_INVALID_FMT);
            }
            else if(x<e->len && f[x]=='$') {    /* hexadecimal numbers */
                if(ftype<0) ftype=FMT_TYP_HEX; else longjmp(shell,ERROR_INVALID_FMT);
            }
            else if(x<e->len && f[x]=='%') {    /* binary numbers */
                if(ftype<0) ftype=FMT_TYP_BIN; else longjmp(shell,ERROR_INVALID_FMT);
            }
            else if(x<e->len && f[x]=='*') {    /* text */
                if(ftype<0) ftype=FMT_TYP_TEXT; else longjmp(shell,ERROR_INVALID_FMT);
                if(++x>=e->len) longjmp(shell,ERROR_INVALID_FMT);
                fillchr=f[x];
            }
            else if(x<e->len && f[x]=='<') {    /* left aligned */
                if(ftype<0 || (fflag&(FMT_FLG_LALGN | FMT_FLG_RALGN | FMT_FLG_CENTRE))) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_LALGN;
            }
            else if(x<e->len && f[x]=='>') {    /* right aligned */
                if(ftype<0 || (fflag&(FMT_FLG_LALGN | FMT_FLG_RALGN | FMT_FLG_CENTRE))) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_RALGN;
            }
            else if(x<e->len && f[x]=='^') {    /* centred */
                if(ftype<0 || (fflag&(FMT_FLG_LALGN | FMT_FLG_RALGN | FMT_FLG_CENTRE))) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_CENTRE;
            }
            else if(x<e->len && f[x]=='+') {    /* forced + sign */
                if(ftype!=FMT_TYP_DEC) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_PLUS;
            }
            else if(x<e->len && f[x]=='-') {    /* reserved space for sign */
                if(ftype!=FMT_TYP_DEC) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_RSVSGN;
            }
            else if(x<e->len && f[x]=='=') {    /* enable leading or trailing zeros */
                if(ftype==FMT_TYP_TEXT) longjmp(shell,ERROR_INVALID_FMT);
                if(!len && !(fflag&FMT_FLG_LP)) fflag|=FMT_FLG_LZERO;
                else if(fflag&FMT_FLG_LP) fflag|=FMT_FLG_TZERO;
                else longjmp(shell,ERROR_INVALID_FMT);
            }
            else if(x<e->len && f[x]=='.') {    /* decimal fraction point */
                if(ftype!=FMT_TYP_DEC || (fflag&FMT_FLG_LP)) longjmp(shell,ERROR_INVALID_FMT);
                fflag|=FMT_FLG_LP;
            }
            else if(x<e->len && f[x] && strchr("0123456789",f[x])) {    /* length specifier */
                if(ftype<0) longjmp(shell,ERROR_INVALID_FMT);
                if(fflag&FMT_FLG_LP) frlen=(10*frlen)+(f[x]-'0'); else len=(10*len)+(f[x]-'0');
            }
            else {  /* continue or invalid data type specifier */
                if(ftype>=0 || (fflag&~(FMT_FLG_SPEC | FMT_FLG_STORE))) {
                    int l=len+frlen+!!(frlen);          /* if (frlen) is not 0, adding one more character for the decimal point */
                    f_data_t *p=get_data(stid,param);   /* get the parameter */
                    if(!p) longjmp(shell,ERROR_STACK_EMPTY);
                    if(((p->id)&TYPE_MASK)==TYPE_TEXT) {  /* the parameter is text */
                        if(ftype!=FMT_TYP_TEXT) longjmp(shell,ERROR_TYPE_MISMATCH);    /* the parameter is text but a number is expected */
                        int px=0;               /* text is left aligned by default */
                        if(l && p->len<l) {     /* try to align */
                            if(fflag&FMT_FLG_RALGN) px=l-p->len;
                            else if(fflag&FMT_FLG_CENTRE) px=(l-p->len)/2;
                        }
                        if(!l) l=p->len;
                        set_data(stid,TYPE_TEXT,0,vs,(v->len+l));   /* increase the length of (v) */
                        v=get_text(0,&vs);      /* reload the output string */
                        e=get_text(1,&f);       /* reload the format string */
                        p=get_data(stid,param); /* reload the parameter */
                        unsigned char *c=&vs[v->len-l];
                        unsigned char *cs=c;
                        while(l--) *(c++)=fillchr;
                        memcpy(cs+px,(unsigned char *)p+HDR,p->len);
                    } else {    /* the parameter is a number */
                        REAL num=rrec_dbl(p);
                        char buf[65] = {0};
                        char fbuf[25] = {0};
                        if(ftype==FMT_TYP_DEC) {
                            *fbuf='%';
                            if(fflag&FMT_FLG_LP) sprintf(&fbuf[strlen(fbuf)],"%u.%uf",len,frlen); else sprintf(&fbuf[strlen(fbuf)],"13.10f");
                            sprintf(buf,fbuf,num);
                            while(buf[strlen(buf)-1]==' ') buf[strlen(buf)-1]=0;    /* cut trailing spaces */
                            while(*buf==' ') memmove(buf,buf+1,strlen(buf));        /* cut leading spaces */
                            if((fflag&FMT_FLG_LP) && !(fflag&FMT_FLG_TZERO)) {
                                while(buf[strlen(buf)-1]=='0') buf[strlen(buf)-1]=0;
                                if(buf[strlen(buf)-1]=='.') buf[strlen(buf)-1]=0;
                            }
                            if(!(fflag&(FMT_FLG_LALGN | FMT_FLG_CENTRE))) fflag|=FMT_FLG_RALGN;     /* numbers are left aligned by default */
                            if((fflag&FMT_FLG_PLUS) && num<0) fflag&=~FMT_FLG_PLUS;
                        }
                        else if(ftype==FMT_TYP_HEX) {
                            unsigned long ni=(unsigned long)num;
                            sprintf(buf,"%lx",(unsigned long)ni);
                            if(!(fflag&(FMT_FLG_LALGN | FMT_FLG_CENTRE))) fflag|=FMT_FLG_RALGN;     /* numbers are left aligned by default */
                        }
                        else if(ftype==FMT_TYP_BIN) {
                            strcpy(buf,bin((unsigned long)num));
                            if(!(fflag&(FMT_FLG_LALGN | FMT_FLG_CENTRE))) fflag|=FMT_FLG_RALGN;     /* numbers are left aligned by default */
                        }
                        int bl=strlen(buf);
                        int px=0;
                        if(l && bl<l) {     /* try to align */
                            if(fflag&FMT_FLG_RALGN) px=l-bl;
                            else if(fflag&FMT_FLG_CENTRE) px=(l-bl)/2;
                        }
                        if(l<bl) l=bl;
                        if(fflag&(FMT_FLG_RSVSGN | FMT_FLG_PLUS)) {
                            l++;
                            px++;
                        }
                        set_data(stid,TYPE_TEXT,0,vs,(v->len+l));   /* increase the length of (v) */
                        v=get_text(0,&vs);      /* reload the output string */
                        e=get_text(1,&f);       /* reload the format string */
                        unsigned char *c=&vs[v->len-l];
                        unsigned char *cs=c;
                        while(l--) *(c++)=' ';
                        memcpy(cs+px,buf,bl);
                        if(fflag&FMT_FLG_LZERO) {
                            char mf=(cs[px]=='-');
                            px+=mf;
                            while(--px>=0 && (cs[px]==' ' || cs[px]=='-')) cs[px]='0';
                            if(mf) *cs='-';
                        }
                        if(fflag&FMT_FLG_RSVSGN) {
                            *cs=' ';
                        }
                        if(fflag&FMT_FLG_PLUS) {
                            while(*(cs+1)==' ') cs++;
                            *cs='+';
                        }
                    }
                    fflag=0;
                    if(x<e->len) fflag|=FMT_FLG_STORE;  /* this last character is not part of the specifier so it just go to the output */
                    len=frlen=0;
                    ftype=-1;
                    param--;
                } else {
                    longjmp(shell,ERROR_INVALID_FMT);
                }
            }
        } else {    /* if not specifier then it is a character for storing */
            fflag|=FMT_FLG_STORE;
        }
        if(x<e->len && (fflag&FMT_FLG_STORE)) { /* storing characters to the output */
            set_data(stid,TYPE_TEXT,0,vs,(v->len+1));   /* increase the length of (v) by one character */
            v=get_text(0,&vs);  /* reload the output string */
            e=get_text(1,&f);   /* reload the format string */
            vs[v->len-1]=f[x];  /* add the current character from (s) */
            fflag&=~(FMT_FLG_SPEC|FMT_FLG_STORE);
        }
        x++;    /* go to the next character in the format string */
    }
    while(stx--) set_data(stid,TYPE_MASK,1,0,-1);   /* do cleanup */
}


void f_system(void) {
    unsigned char *s;
    f_data_t *e=get_text(0,&s);     /* OS command string */
    set_data(stid,TYPE_TEXT,0,s,(e->len+1));    /* increase the length of (e) by one character */
    e=get_text(0,&s);   /* reload (e) because its address might have changed */
    s[e->len-1]=0;      /* assert a properly terminated string for passing to the OS */
    if(_f_sysstr) _f_sysstr((char *)s);
    set_data(stid,TYPE_MASK,0,0,-1);  /* remove the parameter from the stack */
}


void f_run(void) {
    char tn[NAMESPACE_SIZE] = {0};  /* namespace buffer */
    const unsigned char *c=Fmem;
    char skipf;
    while(c<pCode) {
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */

        /* start of a new word detected */
        if(a->code==code[CODE_NEW]) {
            const unsigned char *w=c+1;
            while(w<pCode && *w!=':') w++;
            if(w>=pCode) {  /* code has finished before the end of the word */
                new_line();
                longjmp(shell,ERROR_UNEXPECTED_END);
            }
            int wl=w-c;
            if(wl<2) {  /* the word has invalid name */
                new_line();
                longjmp(shell,ERROR_UNKNOWN_WORD);
            }
            if((strlen(tn)+wl+2)>=NAMESPACE_SIZE) { /* too long namespace */
                new_line();
                longjmp(shell,ERROR_NAME_TOO_LONG);
            }
            if(!(*tn) && memcmp(c+1,"ERROR:",wl)) runPC=(unsigned char *)(w+1);
            tn[strlen(tn)+wl]=0;
            memcpy(&tn[strlen(tn)],c+1,wl);
        }

        /* end of a new word detected */
        else if(a->code==code[CODE_EOW]) {
            if(*tn) {
                do {    /* remove the last word from the namespace */
                   tn[strlen(tn)-1]=0;
                } while(*tn && tn[strlen(tn)-1]!=':');
            }
            /* else longjmp(shell,ERROR_NAMESPACE_OUT); */    /* out of any namespace */
        }

        else if(a->code==find_atom("%text",0)->code) skipf=1;
        else if(a->code==find_atom("%skip",0)->code) skipf=1;

        /* jump to the next pseudocode (still on the current one now) */
        if(a->len==-1) {    /* new word */
            c++;    /* skip the pseudocode */
            while(c<pCode && *c!=':') c++;  /* skip everything until : */
            if(c>=pCode) {  /* code has finished before the end of the word */
                new_line();
                longjmp(shell,ERROR_UNEXPECTED_END);
            }
            c++;    /* skip the ending : */
        }
        else if(a->len==0) {    /* VLN */
            unsigned long v;
            c=get_vln((unsigned char *)(c+1),&v);
            if(skipf) c+=v;
        } else  /* normal situation */
            c+=a->len;

    }
}


/* ============================================================================================================================ */
/* in order to make these words work, file support must be implemented in the platform */

void __attribute__ ((weak)) f_openf(void) {
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]);    /* search for free file handler */
    if(fh<0) longjmp(shell,ERROR_MANY_FILES);
    unsigned char *s;
    f_data_t *e=get_text(0,&s);     /* filename */
    set_data(stid,TYPE_TEXT,0,s,(e->len+1));        /* increase the length of (e) by one character */
    e=get_text(0,&s);               /* reload (e) because its address might have changed */
    s[e->len-1]=0;                  /* assert a properly terminated string for passing to the C function */
    FILE *f=fopen((char *)s,"a+b");
    long l=-1;
    if(f) {
        fseek(f,0,SEEK_END);
        l=ftell(f);
        rewind (f);
        files[fh]=f;    /* assign this file handler */
    }
    set_data(stid,TYPE_MASK,0,0,-1);          /* remove the input parameter */
    if(f) set_num(stid,TYPE_SINT,-1,(REAL)((unsigned long)f)); else set_num(stid,TYPE_SINT,-1,-1.0);    /* insert the file handler (returning the numeric value of the pointer to be used as a unique token) */
    set_num(stid,TYPE_SINT,-1,(REAL)l);     /* insert the file length */
}


void __attribute__ ((weak)) f_closef(void) {
    long fh=MAX_FILES;
    FILE *ftkn=(FILE *)((unsigned long)get_num(0));
    if(ftkn) {
        while(--fh>=0 && files[fh]!=ftkn);
        if(fh<0) longjmp(shell,ERROR_UNKNOWN_FILE);
        fclose(ftkn);
        files[fh]=0;    /* release this file handler */
    } else {
        while(--fh>=0) {
            if(files[fh]) {
                fclose(files[fh]);
                files[fh]=0;
            }
        }
    }
}


void __attribute__ ((weak)) f_eoff(void) {
    FILE *ftkn=(FILE *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    set_num(stid,TYPE_SINT,-1,(REAL)(!!feof(ftkn)));
}


void __attribute__ ((weak)) f_seekf(void) {
    unsigned long pos=(unsigned long)get_num(0);
    FILE *ftkn=(FILE *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    if(!fseek(ftkn,pos,SEEK_SET)) set_num(stid,TYPE_SINT,-1,(REAL)ftell(ftkn)); else set_num(stid,TYPE_SINT,-1,-1.0);
}


void __attribute__ ((weak)) f_posf(void) {
    FILE *ftkn=(FILE *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    set_num(stid,TYPE_SINT,-1,(REAL)ftell(ftkn));
}


void __attribute__ ((weak)) f_fout(void) {
    FILE *ftkn=(FILE *)((unsigned long)get_num(1));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    unsigned char *c=((unsigned char *)e+HDR);
    unsigned long l=0;
    if(((e->id)&TYPE_MASK)==TYPE_TEXT) {
        l=fwrite((unsigned char *)c,1,e->len,ftkn);
    } else {
        REAL v=rrec_dbl(e);
        char buf[25] = {0};
        sprintf(buf,"%.10g",(double)v);
        l=fwrite((unsigned char *)buf,1,strlen(buf),ftkn);
    }
    set_num(stid,TYPE_UINT,0,(REAL)l);  /* convert the top element (which was the input data) into number of written characters output value */
}


void __attribute__ ((weak)) f_fin(void) {
    unsigned long l=get_num(0);
    if((pData-pCode)<=(2*HDR)) longjmp(shell,ERROR_NO_MEMORY);
    FILE *ftkn=(FILE *)((unsigned long)get_num(0));
    long fh=MAX_FILES;
    while(--fh>=0 && files[fh]!=ftkn);
    if(fh<0 || !ftkn) longjmp(shell,ERROR_UNKNOWN_FILE);
    char *buf=(char *)(pCode+64);
    set_data(stid,TYPE_TEXT,-1,buf,l);
    l=fread((unsigned char *)buf,1,l,ftkn);
    set_data(stid,TYPE_TEXT,0,buf,l);   /* adjust the length according to the number of actually read characters */
}


/* ============================================================================================================================ */
/* these words rely entirely on helper functions from outside and written for the particular platform */

void fanf_restore(void) {
    if(_fanf_restore) {
        _fanf_restore();
        if(flags&FLAG_RESTR_OK) longjmp(shell,ERROR_NONE); else flags|=FLAG_RESTR_OK;
    }
}


void fanf_store(void) {
    if(_fanf_store) _fanf_store();
}


/* ============================================================================================================================ */
/* NOTE: the functions 'f_.....' are true atomic words intended for use in code, but the functions 'fanf_.....' are shell words */

/* built-in FANF atomic words; a FANF program is compiled to FVM-code consisting of these words only */
const f_atom_t atom[] = {

/* ---------------------------------------------------------------------------------------------------------------------------- */
/* this group contains special words which only appear in the compiled F-code */
/* DO NOT CHANGE THE NAME OR CODE FOR ANY OF THESE */

    {"%nop",    W_ATOM, 0x00,   1,  0},             /* no parameters */
                                                    /* no operation (used to mark an end of a source line) */

    {"%new",    W_ATOM, 0x01,   -1, f_new},         /* data(int8), data(int8), ..., ':' */
                                                    /* [word name in ASCII] */

    {";",       W_ATOM, 0x02,   1,  f_eow},         /* no parameters */
                                                    /* end of a definition */

    {"%call",	W_ATOM, 0x03,   0,  f_call},        /* address(VLN) */
                                                    /* [absolute address within the program space] */

    {"%task",	W_ATOM, 0x04,   0,  f_task},        /* address(VLN) */
                                                    /* [absolute address within the program space] */

    {"%ref",	W_ATOM, 0x05,	0,  f_ref},         /* address(VLN) */
                                                    /* [absolute address within the program space] */

    {"%nfmt",   W_ATOM, 0x06,   2,  0},             /* numformat(signed int8) */
                                                    /* [0:hexadecimal, 1:binary] */
                                                    /* this pseudocode is purely for purpose of lising source back */
                                                    /* it deterines the listing format of the following integer constant */
                                                    /* if missing, integer numbers are always listed in decimal format */

    {"%sint8",	W_ATOM, 0x07,   2,  f_sint8},       /* value(signed int8) */
                                                    /* [value] */

    {"%uint8",	W_ATOM, 0x08,   2,  f_uint8},       /* value(unsigned int8) */
                                                    /* [value] */

    {"%sint16",	W_ATOM, 0x09,   3,  f_sint16},      /* value(signed int16) */
                                                    /* [value] */

    {"%uint16",	W_ATOM, 0x0a,   3,  f_uint16},      /* value(unsigned int16) */
                                                    /* [value] */

    {"%sint32",	W_ATOM, 0x0b,   5,  f_sint32},      /* value(signed int32) */
                                                    /* [value] */

    {"%uint32",	W_ATOM, 0x0c,   5,  f_uint32},      /* value(unsigned int32) */
                                                    /* [value] */

    {"%real",	W_ATOM, 0x0d,   1+sizeof(REAL),f_real}, /* value(REAL) */
                                                    /* [value] */

    {"%text",	W_ATOM, 0x0e,   0,  f_text},        /* length(VLN)  data(int8), data(int8), data(int8), data(int8), ..... */
                                                    /* [length]     [text/data] */

    {"%skip",	W_ATOM, 0x0f,   0,  f_skip},        /* length(VLN)  data(int8), data(int8), data(int8), data(int8), ..... */
                                                    /* [length]     [text/data] */

/* ---------------------------------------------------------------------------------------------------------------------------- */
/* default set of FANF words available to the user begin here */
/* DO NOT CHANGE ANY WORD NAME FROM THIS GROUP; CODES CAN BE CHANGED */

/* **************************************************************************************************************************** */
/* NOTE: code 0x10 is reserved for future code page extension and must not be used */
/* **************************************************************************************************************************** */

    {"=",       W_ATOM, 0x11,	1,  f_set},         /* v @w = */
                                                    /* set an element of @w with value v */

    {"!",       W_ATOM, 0x12,	1,  f_index},       /* x ! */
                                                    /* load the variable index pointer with value */

    {"if",      W_ATOM, 0x14,	1,  f_if},          /* x if */
                                                    /* continue if x is not 0, otherwise jump to the relevant 'else' address */

    {"else",	W_ATOM, 0x15,	1,  f_else},        /* else */
                                                    /* mark the 'else' option for an 'if' section */

    {"endif",	W_ATOM, 0x16,	1,  f_endif},       /* endif */
                                                    /* mark the end of an 'if' section */

    {"do",      W_ATOM, 0x18,	1,  f_do},          /* do */
                                                    /* mark the start of an 'repeat' section */

    {"while",	W_ATOM, 0x19,	1,  f_while},       /* @x while */
                                                    /* execute the section while ... repeat if @x is not 0 */

    {"loop",	W_ATOM, 0x1b,	1,  f_loop},        /* loop */
                                                    /* execute the section loop ... repeat exactly x times */

    {"repeat",	W_ATOM, 0x1c,	1,  f_repeat},      /* different formats according to the loop opening statement */
                                                    /* do ..... x repeat            (if x is not 0, return back to the relevant 'do') */
                                                    /* @x while ........ repeat     (unconditionally return to 'while'; works with variables only */
                                                    /* x loop ..... repeat          (repeats the loop x times) */

    {"cont",    W_ATOM, 0x1e,	1,  f_cont},        /* cont */
                                                    /* continue the execution of the current 'do', 'while' or 'if' structure skipping all the remaining words before the closing statement */

    {"break",	W_ATOM, 0x1f,	1,  f_break},       /* break */
                                                    /* break the execution of the current 'do', 'while' or 'if' structure */

/* ---------------------------------------------------------------------------------------------------------------------------- */
/* PARAMETERS CAN BE CHANGED AT WILL */

    {"size",	W_ATOM, 0x20,	1,  f_vsize},       /* @w size */
                                                    /* return the size of a data container */

    {"data",	W_ATOM, 0x21,	1,  f_data},        /* n data */
                                                    /* reserve n data cells associated with the current word */

    {"clear",	W_ATOM, 0x22,	1,  f_clear},       /* clear */
                                                    /* clear the entire data memory - stack for all processes and data containers */

    {"empty",	W_ATOM, 0x23,	1,  f_empty},       /* empty */
                                                    /* empty the stack for the current process */

    {"end",     W_ATOM, 0x24,   1,  f_end},         /* end */
                                                    /* prematurely exit the current word */

    {"endall",  W_ATOM, 0x25,   1,  f_endall},      /* endall */
                                                    /* ends all active threads at once */

    {"run",     W_ATOM, 0x26,   1,  f_run},         /* run */
                                                    /* run the last top level word */

    {"depth",	W_ATOM, 0x27,	1,  f_depth},       /* depth */
                                                    /* return the current depth of the stack */
                                                    /* (top) = number of elements currently in the stack */

    {"drop",	W_ATOM, 0x28,	1,  f_drop},        /* drop */
                                                    /* remove the top stack element */

    {"dup", 	W_ATOM, 0x29,	1,  f_dup},         /* x dup */
                                                    /* duplicate the top stack element */

    {"copy",	W_ATOM, 0x2a,	1,  f_copy},        /* n copy */
                                                    /* copy the n-th element in the stack to the top */
                                                    /* depth indexes start from 0, so '0 copy' has the same effect as 'dup' */
                                                    /* negative depth indexes have no effect */

    {"swap",	W_ATOM, 0x2b,	1,  f_swap},        /* n swap */
                                                    /* swap the n-th element in the stack with the top (does not change the number of elements in the stack) */
                                                    /* depth indexes start from 0, so '0 swap' has no effect */
                                                    /* negative depth indexes have no effect */

    {"type",	W_ATOM, 0x2c,	1,  f_type},        /* x type */
                                                    /* return the type of element x */
                                                    /* (top) = (0:number; 1:text) */

    {"astext",	W_ATOM, 0x2d,	1,  f_astext},      /* x astext */
                                                    /* (top) = x converted into text */
                                                    /* if x is a number, it is first converted into its text representation; no operation is done if x is text */

    {"asnum",	W_ATOM, 0x2e,	1,  f_asnum},       /* x asnum */
                                                    /* (top) = x converted into number */
                                                    /* if x is text, it is first converted into number (result 0 on error); no operation is done if x is already a number */

    {"isnum",   W_ATOM, 0x2f,	1,  f_isnum},       /* x isnum */
                                                    /* (top) = 1 if x can be converted into a number, 0 otherwise */

    {"M8w",     W_ATOM, 0x32,	1,  f_m8w},         /* x addr M8w */
                                                    /* write 8-bit unsigned x to physical memory address addr */
                                                    /* works with integer numbers */

    {"M8r",     W_ATOM, 0x33,	1,  f_m8r},         /* addr M8r */
                                                    /* (top) = read 8-bit unsigned from physical memory address addr */
                                                    /* works with integer numbers */

    {"M16w",	W_ATOM, 0x34,	1,  f_m16w},        /* x addr M16w */
                                                    /* write 16-bit unsigned x to physical memory address addr */
                                                    /* works with integer numbers */

    {"M16r",	W_ATOM, 0x35,	1,  f_m16r},        /* addr M16r */
                                                    /* (top) = read 16-bit unsigned from physical memory address addr */
                                                    /* works with integer numbers */

    {"M32w",	W_ATOM, 0x38,	1,  f_m32w},        /* x addr M32w */
                                                    /* write 32-bit unsigned x to physical memory address addr */
                                                    /* works with integer numbers */

    {"M32r",	W_ATOM, 0x39,	1,  f_m32r},        /* addr M32r */
                                                    /* (top) = read 32-bit unsigned from physical memory address addr */
                                                    /* works with integer numbers */

    {"not", 	W_ATOM, 0x40,	1,  f_not},         /* x not */
                                                    /* (top) = not x */
                                                    /* bitwise operation on an integer number */

    {"and",     W_ATOM, 0x41,	1,  f_and},         /* x y and */
                                                    /* (top) = x and y */
                                                    /* bitwise operation on integer numbers */

    {"or",      W_ATOM, 0x42,	1,  f_or},          /* x y or */
                                                    /* (top) = x or y */
                                                    /* bitwise operation on integer numbers */

    {"xor",     W_ATOM, 0x43,	1,  f_xor},         /* x y xor */
                                                    /* (top) = x xor y */
                                                    /* bitwise operation on integer numbers */

    {"shl",     W_ATOM, 0x44,	1,  f_shl},         /* x y shl */
                                                    /* (top) = x shifted left y times (LSB=0)*/
                                                    /* bitwise operation on integer numbers */

    {"shr",     W_ATOM, 0x45,	1,  f_shr},         /* x y shr */
                                                    /* (top) = x shifted right y times (MSB=0)*/
                                                    /* bitwise operation on integer numbers */

    {"?",       W_ATOM, 0x47,	1,  f_is},          /* x ? */
                                                    /* return 1 if x contains any data of any type, 0 if x contains no data */

    {"==",      W_ATOM, 0x48,	1,  f_teq},         /* x y == */
                                                    /* (top) = 1:(x is the same as y); 0:(x is not the same as y) */
                                                    /* works with any data type */

    {"<>",      W_ATOM, 0x49,	1,  f_tne},         /* x y <> */
                                                    /* (top) = 1:(x is not the same as y); 0:(x is the same as y) */
                                                    /* works with any data type */

    {">",       W_ATOM, 0x4a,	1,  f_tgr},         /* x y > */
                                                    /* (top) = 1:(x > y); 0:(x <= y) */
                                                    /* works with numbers; for text compares the length; with different data types returns always 0 */

    {">=",      W_ATOM, 0x4b,	1,  f_tge},         /* x y >= */
                                                    /* (top) = 1:(x >= y); 0:(x < y) */
                                                    /* works with numbers; for text compares the length; with different data types returns always 0 */

    {"<",       W_ATOM, 0x4c,	1,  f_tsm},         /* x y < */
                                                    /* (top) = 1:(x < y); 0:(x >= y) */
                                                    /* works with numbers; for text compares the length; with different data types returns always 0 */

    {"<=",      W_ATOM, 0x4d,	1,  f_tse},         /* x y <= */
                                                    /* (top) = 1:(x <= y); 0:(x > y) */
                                                    /* works with numbers; for text compares the length; with different data types returns always 0 */

    {"sign",	W_ATOM, 0x50,	1,  f_sign},        /* x sign */
                                                    /* (top) = 1:(if x is positive or 0), -1:(if x is negative) */
                                                    /* works with numbers only; for text types always returns 1 */

    {"+",       W_ATOM, 0x51,	1,  f_add},         /* x y + */
                                                    /* (top) = x+y */

    {"-",       W_ATOM, 0x52,	1,  f_sub},         /* x y - */
                                                    /* (top) = x-y */

    {"*",       W_ATOM, 0x53,	1,  f_mul},         /* x y * */
                                                    /* (top) = x*y */

    {"/",       W_ATOM, 0x54,	1,  f_div},         /* x y / */
                                                    /* (top) = x/y */

    {"//",      W_ATOM, 0x55,	1,  f_mod},         /* x y // */
                                                    /* (top) = modulo(x/y) */
                                                    /* by its definition applicable to integer numbers only */

    {"_+1",     W_ATOM, 0x56,	1,  f_inc},         /* x _+1 */
                                                    /* the value on the top of the stack is increased by 1; text type data will generate an error */
                                                    /* (top) = (top)+1 */

    {"_-1",     W_ATOM, 0x57,	1,  f_dec},         /* x _-1 */
                                                    /* the value on the top of the stack is decreased by 1; text type data will generate an error */
                                                    /* (top) = (top)-1 */

    {"abs",     W_ATOM, 0x58,	1,  f_abs},         /* x abs */
                                                    /* return the absolute value of x */

    {"round",	W_ATOM, 0x59,	1,  f_round},       /* x round */
                                                    /* round the value of x to the nearest integer */

    {"trim",	W_ATOM, 0x5a,	1,  f_trim},        /* x trim */
                                                    /* trim the value of x to integer */

    {"power",	W_ATOM, 0x5b,	1,  f_power},       /* x y power */
                                                    /* calculate x^y */

    {"sin",     W_ATOM, 0x5c,	1,  f_sin},         /* x sin */
                                                    /* (top) = sin(x) */

    {"cos",     W_ATOM, 0x5d,	1,  f_cos},         /* x cos */
                                                    /* (top) = cos(x) */

    {"tan",     W_ATOM, 0x5e,	1,  f_tan},         /* x tan */
                                                    /* (top) = tan(x) */

    {"atan",	W_ATOM, 0x5f,	1,  f_atan},        /* x atan */
                                                    /* (top) = atan(x) */

    {"rad",     W_ATOM, 0x60,	1,  f_rad},         /* x rad */
                                                    /* convert x from degrees into radians */

    {"deg",     W_ATOM, 0x61,	1,  f_deg},         /* x deg */
                                                    /* convert x from radians into degrees */

    {"ln",      W_ATOM, 0x62,	1,  f_ln},          /* x ln */
                                                    /* calculate natural logarithm of x */

    {"exp", 	W_ATOM, 0x63,	1,  f_exp},         /* x exp */
                                                    /* calculate natural exp of x */

    {"log",     W_ATOM, 0x64,	1,  f_log},         /* x log */
                                                    /* calculate decimal logarithm of x */

    {"rnd",     W_ATOM, 0x65,	1,  f_rnd},         /* rnd */
                                                    /* return a random number between 0 and 1 */

    {"rndseed",	W_ATOM, 0x66,	1,  f_rndseed},     /* x rndseed */
                                                    /* initialise the pseudo-random generator with seed x */

    {"PI",      W_ATOM, 0x67,   1,  f_PI},          /* PI */
                                                    /* return the pi constant */

    {"E",       W_ATOM, 0x68,   1,  f_E},           /* E */
                                                    /* return the e constant */

    {"len",     W_ATOM, 0x70,	1,  f_len},         /* x len */
                                                    /* (top) = length of x */
                                                    /* works with text type; for numbers the length is the number of characters as if the number if output to the console */

    {"++",      W_ATOM, 0x71,	1,  f_concat},      /* x y ++ */
                                                    /* concatenate x and y */
                                                    /* works with text type */

    {"cut",     W_ATOM, 0x72,	1,  f_cut},         /* s b c cut */
                                                    /* (top) = cut c characters starting from the b-th (b starts from 0) from s */
                                                    /* works with text data only */

    {"delete",	W_ATOM, 0x73,	1,  f_delete},      /* s b c delete */
                                                    /* delete c characters starting from the b-th (b starts from 0) from s */
                                                    /* works with text data only */

    {"insert",	W_ATOM, 0x74,	1,  f_insert},      /* d s x insert  */
                                                    /* insert text s into d starting from position x */
                                                    /* works with text data only */

    {"replace",	W_ATOM, 0x75,	1,  f_replace},     /* d s x replace  */
                                                    /* replace a fragment in d with the text s starting from position x */
                                                    /* the length of d is increased if necessary */
                                                    /* works with text data only */

    {"scan",	W_ATOM, 0x76,	1,  f_scan},        /* d s x scan */
                                                    /* scan d starting from the x-position and return the first occurrence of s or -1 if not found */

    {"format",	W_ATOM, 0x77,	1,  f_format},      /* [... x] f format */
                                                    /* format input parameters according to the format specifier(s) f and return the result in text form */
                                                    /* format specifiers follow the pattern: |type[modifiers][[=]length[.fraction[=]]] */
                                                    /* specifiers: # (decimal number), $ (hexadecimal number), % (binary number), | (the | character) */
                                                    /* specifiers: * (text); the * character MUST be followed by a fill character for the text */
                                                    /* modifiers: < (left aligned), > (right aligned), ^ (centred), */
                                                    /* modifiers: + (forced + sign for decimal numbers), - (reserved space for sign) */
                                                    /* = (enable leading or trailing zeros in numbers) */
                                                    /* fractional length is applicable to decimal numbers only */
                                                    /* a format string f can contain text literals and any number of parameters to be taken from stack */

    {"open",	W_ATOM, 0x80,	1,  f_openf},       /* n open */
                                                    /* open file with name n */
                                                    /* this function returns TWO values in the stack */
                                                    /* (top-1) = file handler or -1 on failure */
                                                    /* (top) = current file length in bytes or -1 on failure */

    {"close",	W_ATOM, 0x81,	1,  f_closef},      /* h close */
                                                    /* close file with handler h */

    {"eof",     W_ATOM, 0x82,	1,  f_eoff},        /* h eof */
                                                    /* return 1 if the end of file has been reached, and 0 otherwise */

    {"seek",	W_ATOM, 0x83,	1,  f_seekf},       /* h x seek */
                                                    /* place the internal file position pointer at offset x from the beginning of the file */
                                                    /* return the actual position of the pointer or -1 on failure or EOF */

    {"pos",     W_ATOM, 0x84,	1,  f_posf},        /* h pos */
                                                    /* return the current file pointer position in the file; return -1 on failure or EOF */

    {"fout",    W_ATOM, 0x8e,	1,  f_fout},        /* h x fout */
                                                    /* (top) ---> (file h) */
                                                    /* text data is output as it is while number type is converted into text and then output */
                                                    /* if the stack is empty, nothing is output */
                                                    /* will return the number of actually sent characters */

    {"fin",     W_ATOM, 0x8f,	1,  f_fin},         /* h x fin */
                                                    /* (x readings from file h) ---> (top) */
                                                    /* read exactly x characters from h into a text type element */

    {"nwcin",   W_ATOM, 0x9d,	1,  f_nwcin},       /* x nwcin */
                                                    /* (x readings from console without waiting) ---> (top) */
                                                    /* read exactly x characters from the console into a text type element */
                                                    /* if the currently available characters in the console buffer are less than x, the word will not wait and will return only what is read */

    {"cout",	W_ATOM, 0x9e,	1,  f_cout},        /* x cout */
                                                    /* (top) ---> (console) */
                                                    /* text data is output as it is; numeric type is first converted into text */

    {"cin",     W_ATOM, 0x9f,	1,  f_cin},         /* cin */
                                                    /* (console as text input) ---> (top) */
                                                    /* read the console until a LF character; editing with backspace is allowed */
                                                    /* will return a text type element */

    {"source",  W_ATOM, 0xa0,   1,  f_source},      /* s source */
                                                    /* compile (and execute if necessary) provided FANF source code */
                                                    /* NOTE: the source text MUST be properly terminated by a NULL, LF, CR or space character */

    {"help",    W_SHELL,0xa1,   1,  fanf_help},     /* help */
                                                    /* basic help information */

    {"isknown", W_ATOM, 0xa2,   1,  f_isknown},     /* w isknown */
                                                    /* return 1 if the word 'w' is an atomic word, 2 if the word already known in the library, or 0 otherwise */

    {"use",     W_ATOM, 0xa3,	1,  f_use},         /* w use */
                                                    /* use a defined word w passed as its text name */

    {"forget",	W_ATOM, 0xa4,	1,  f_forget},      /* w forget */
                                                    /* remove the definitions of all defined words starting from w until the most recent one */

    {"restore", W_SHELL,0xa5,   1,  fanf_restore},  /* restore */
                                                    /* restore the library from NVM */

    {"store",   W_SHELL,0xa6,   1,  fanf_store},    /* store */
                                                    /* store the library into NVM */

    {"threads",	W_ATOM, 0xa7,	1,  f_threads},     /* threads */
                                                    /* return the number of currently active threads */

    {"maxthds", W_ATOM, 0xa8,   1,  f_maxthreads},  /* maxthds */
                                                    /* return the maximum number of supported simultaneous threads */

    {"free",	W_ATOM, 0xa9,	1,  f_free},        /* free */
                                                    /* return the amount of currently free memory (program plus data) in bytes */

    {"system",	W_ATOM, 0xaa,	1,  f_system},      /* x system */
                                                    /* pass the text 'x' to the operating system for execution */
                                                    /* (not available in all systems) */

    {"machine",	W_ATOM, 0xab,	1,  f_machine},     /* x machine */
                                                    /* execute the text x as piece of machine code */
                                                    /* (not available in all systems) */

    {"words",   W_SHELL,0xac,   1,  fanf_words},    /* words */
                                                    /* list all words from the dictionary */

    {"peek",    W_SHELL,0xad,   1,  fanf_peek},     /* peek */
                                                    /* display all stack data */
                                                    /* NOTE: does not remove anything from the stack */

    {"insp",    W_SHELL,0xae,   1,  fanf_insp},     /* w insp */
                                                    /* inspect a given word or if no parameter is supplied - all data containers */

    {"resrc",   W_SHELL,0xaf,   1,  fanf_resrc},    /* x s resrc */
                                                    /* replace source line starting from index x (given by the 'list' word) with the new source s */

    {"insrc",   W_SHELL,0xb0,   1,  fanf_insrc},    /* x s insrc */
                                                    /* insert a new source line s starting from index x (given by the 'list' word) */

    {"list",    W_SHELL,0xb1,	1,  fanf_list},     /* w list */
                                                    /* list a given word or if no parameter is supplied - the entire library */

#ifdef DEBUG
    {"dump",    W_SHELL,0xb2,	1,  fanf_dump},     /* a l dump */
                                                    /* hexadecimal dump of l bytes starting from address a (mainly for debug) */
#endif

/* ---------------------------------------------------------------------------------------------------------------------------- */
/* PLATFORM-DEPENDENT WORDS */
/* occupy codes 0xc0 through 0xff and in future - codes 0x100 through 0x1ff */

#ifdef MORE_WORDS_FILE
#define MORE_WORDS_DEF
#include MORE_WORDS_FILE_INCLUDE(MORE_WORDS_FILE)
#undef MORE_WORDS_DEF
#endif

{0}};
