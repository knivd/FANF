#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shellw.h"
#include "fanf.h"
#include "memory.h"
#include "atoms.h"


/*
basic help information
*/
void fanf_help(void) {
    f_author(0);
#ifndef DISABLE_HELP
    new_line();
    printf("Basic FANF syntax:"); new_line();
    printf("ground rule: FANF uses 'Reverse Polish Notation' (RPN)!"); new_line();
    printf("most operations are done through a mixed use stack"); new_line();
    new_line();
    printf("`commentaries can span on more than one line`"); new_line();
    printf("`! this one is only until the end of the current line"); new_line();
    new_line();
    printf("text: \"this is a text constant\""); new_line();
    printf("_#013 _$0d _%%00011101  special characters can be inserted by code"); new_line();
    printf("_=  special characters can be inserted this way as well (code+$30)"); new_line();
    new_line();
    printf("numbers: [#]decimal, $hexadecimal, %%binary"); new_line();
    printf("decimal number format: [sign]base[.fraction][E/e[sign]mantissa]"); new_line();
    printf("examples: -12, $7a8c, %%110101010010, 3.23, -2.7899e-5"); new_line();
    new_line();
    printf("new_word: word word word ..... ;"); new_line();
    printf("where 'word' can be any previously defined word or literal"); new_line();
    printf("allowed is the form   new_word:  new_subword: ... ; ... ;"); new_line();
    printf("calling convention:  word[:subword]...  or  [:]...word"); new_line();
    new_line();
    printf("data container assignment:  data [index !] @word ="); new_line();
    printf("data container access:  [index !] @word"); new_line();
    printf("all indexes start from 0; single data containers have the only index 0"); new_line();
    printf("example:  @MyOldVar 3 ! @MyNewVar =   will set the 3rd element in"); new_line();
    printf("MyNewVar with the data read from the current index (or index 0) in MyOldVar"); new_line();
    new_line();
    printf("control structures:"); new_line();
    printf("<condition> if ......[break]...... else ......[break]..... endif"); new_line();
    printf("do ......[break]...... <condition> repeat"); new_line();
    printf("<count> loop ......[break]...... repeat"); new_line();
    printf("<@word> while ......[break]...... repeat"); new_line();
    new_line();
    fanf_words();
#else
    printf("basic help is not included; use 'words' instead");
    new_line();
#endif
}


/*
list currently compiled FANF words
*/
void fanf_words(void) {
    const signed char wtypes[] = {W_SHELL, W_ATOM, W_EXTRA, -1};
    new_line();
    char col=0;
    int t=0;
    while(wtypes[t]>=0) {
        char pf=0;
        const f_atom_t *a=atom;
        while(a->word) {
            /* print the words belonging to the selected type */
            if(a->type==(unsigned char)wtypes[t] && a->code!=code[CODE_EOW] && *a->word!='%') {
                pf=1;
                printf("%s\t",a->word);
                if(++col>=8) {
                    new_line();
                    col=0;
                }
            }
            a++;
        }
        if(pf) {
            new_line();
            new_line();
        }
        t++;
        col=0;
    }
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
            tn[strlen(tn)+wl]=0;
            memcpy(&tn[strlen(tn)],c+1,wl);
            if(*tn) {
                tn[strlen(tn)-1]=0; /* temporarily remove the ending : character */
                printf("%s\t",tn);  /* print the word */
                if(++col>=4) {
                    new_line();
                    col=0;
                }
                tn[strlen(tn)]=':'; /* restore the : character */
            }
        }

        /* end of a new word detected */
        else if(a->code==code[CODE_EOW]) {
            if(!(*tn)) {    /* expected never to happen because this situation is handled earlier during compilation */
                new_line();
                longjmp(shell,ERROR_NAMESPACE_OUT);    /* out of any namespace */
            }
            do {    /* remove the last word from the namespace */
               tn[strlen(tn)-1]=0;
            } while(*tn && tn[strlen(tn)-1]!=':');
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
    new_line();
    new_line();
    unsigned long total=(unsigned long)(pCode-Fmem);
    printf("user library: %lu byte",total);
    if(total!=1) printf("s");
    new_line();
}


/*
[w] list
list a given word or if no parameter is supplied - the entire library
*/
void fanf_list(void) {
    char wf=0;
    char tn[NAMESPACE_SIZE] = {0};
    const unsigned char *c=Fmem;
    f_data_t *e=get_data(stid,0);
    if(e) {
        if(((e->id)&TYPE_MASK)!=TYPE_TEXT) longjmp(shell,ERROR_TYPE_MISMATCH);
        if((e->len+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
        if(e->len) memcpy(tn,((unsigned char *)e+HDR),e->len);
        set_data(stid,TYPE_MASK,0,0,-1);
        if(*tn) {
            const f_atom_t *a=find_atom(tn,0);
            if(a) longjmp(shell,ERROR_ATOM_LISTING);
            if(*tn && tn[strlen(tn)]!=':') strcat(tn,":");  /* assert a lexically valid namespace */
            c=find_word(tn);
            if(!c) longjmp(shell,ERROR_UNKNOWN_WORD);
            wf=1;
            memset(tn,0,NAMESPACE_SIZE);
        }
    }
    char skipf;
    char nfmt=-1;
    const unsigned char *begin=c;
    fanf_mpf(pCode,MPF_CLEANUP);
    while(c<pCode) {
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */

        if(a->code==code[CODE_NOP]) {
            new_line();
            printf("[%04lx]    ",(unsigned long)(c-Fmem));
        }

        else if(a->code==code[CODE_NEW]) {
            const unsigned char *w=(c+1);
            while(w<pCode && *w!=':') w++;
            if(w>=pCode) {  /* code has finished before the end of the word */
                new_line();
                longjmp(shell,ERROR_UNEXPECTED_END);
            }
            int wl=(w-c);
            if(wl<2) {  /* the word has invalid name */
                new_line();
                longjmp(shell,ERROR_UNKNOWN_WORD);
            }
            if((strlen(tn)+wl+2)>=NAMESPACE_SIZE) { /* too long namespace */
                new_line();
                longjmp(shell,ERROR_NAME_TOO_LONG);
            }
            char *curr=&tn[strlen(tn)];
            tn[strlen(tn)+wl]=0;
            memcpy(curr,c+1,wl);
            if(*tn) {
                new_line();
                printf("        %s",curr);  /* (curr) will display only the word name; (tn) will display the entire namespace for the word */
            }
        }

        else if(a->code==code[CODE_EOW]) {
            char warnf=0;
            if(!(*tn)) warnf=1; /* never expected to happen because this situation is handled earlier during compilation */
            do {    /* remove the last word from the namespace */
               tn[strlen(tn)-1]=0;
            } while(*tn && tn[strlen(tn)-1]!=':');
            printf(";");
            if(warnf) printf("  `>>> WARNING: out of any namespace <<<`");
            new_line();
            if(wf && !(*tn)) break;
        }

        else if(a->code==find_atom("%call",0)->code) {
            unsigned long addr;
            get_vln((unsigned char *)(c+1),&addr);
            print_addr_word(addr,tn);
        }

        else if(a->code==find_atom("%task",0)->code) {
            unsigned long addr;
            get_vln((unsigned char *)(c+1),&addr);
            printf("~");
            print_addr_word(addr,tn);
        }

        else if(a->code==find_atom("%ref",0)->code) {
            unsigned long addr;
            get_vln((unsigned char *)(c+1),&addr);
            printf("@");
            print_addr_word(addr,tn);
        }

        else if(a->code==code[CODE_NFMT]) {
            nfmt=*(c+1);
        }

        else if(a->code==find_atom("%sint8",0)->code) {
            char vi=*(c+1);
            switch(nfmt) {
                default: printf("%i",vi); break;
                case 0: printf("$%x",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%uint8",0)->code) {
            unsigned char vi=*(c+1);
            switch(nfmt) {
                default: printf("%u",vi); break;
                case 0: printf("$%x",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%sint16",0)->code) {
            int vi=0;
            char x;
            for(x=0; x<2; x++) vi=(vi<<8)+*(c+1+x);
            if(vi>32767) vi=-(((vi&0xffff)^0xffff)+1);
            switch(nfmt) {
                default: printf("%i",vi); break;
                case 0: printf("$%x",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%uint16",0)->code) {
            unsigned int vi=0;
            char x;
            for(x=0; x<2; x++) vi=(vi<<8)+*(c+1+x);
            switch(nfmt) {
                default: printf("%u",vi); break;
                case 0: printf("$%x",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%sint32",0)->code) {
            long vi=0;
            char x;
            for(x=0; x<4; x++) vi=(vi<<8)+*(c+1+x);
            if(vi>=0x80000000) vi=-(((vi&0xffffffff)^0xffffffff)+1);
            switch(nfmt) {
                default: printf("%li",vi); break;
                case 0: printf("$%lx",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%uint32",0)->code) {
            unsigned long vi=0;
            char x;
            for(x=0; x<4; x++) vi=(vi<<8)+*(c+1+x);
            switch(nfmt) {
                default: printf("%lu",vi); break;
                case 0: printf("$%lx",vi); break;
                case 1: printf("%%%s",bin(vi)); break;
            }
            nfmt=-1;
        }

        else if(a->code==find_atom("%real",0)->code) {
            REAL v;
            memcpy(&v,c+1,sizeof(REAL));
            printf("%.10g",(double)v);
        }

        else if(a->code==find_atom("%text",0)->code) {
            printf("\"");
            unsigned long v;
            unsigned char *p=get_vln((unsigned char *)(c+1),&v);
            while(v--) {
                if(*p>=' ') printf("%c",*p); else printf("_#%03d",*p);
                p++;
            }
            printf("\"");
            skipf=1;
        }

        else if(a->code==find_atom("%skip",0)->code) {
            printf("`");
            unsigned long v;
            unsigned char *p=get_vln((unsigned char *)(c+1),&v);
            while(v--) {
                if(*p>=' ') printf("%c",*p);
                p++;
            }
            printf("`");
            skipf=1;
        }

        else printf("%s",a->word);

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
        } else {    /* normal situation */
            c+=a->len;
        }

        if(a->code!=code[CODE_NOP] && a->code!=code[CODE_NFMT]) printf("  ");
    }
    new_line();
    new_line();
    unsigned long total=(unsigned long)(c-begin);
    printf("%lu byte",total);
    if(total!=1) printf("s");
    new_line();
}


/*
display all stack data
NOTE: does not change anything in the stack
*/
void fanf_peek(void) {
    unsigned int count=0;
    unsigned long total=0;
    unsigned char *p=pData;
    while((p-Fmem)<FMEM_SIZE) { /* check whether this element already exists in the chain */
        f_data_t *e=(f_data_t *)p;
        if(e->id>=STACK_DATA) {
            count++;
            int z=e->id&(~STACK_DATA);
            printf("[%4u]  ",(z>>2));
            printf("%10u  ",e->len);
            total+=(unsigned int)(e->len);
            if((z&TYPE_MASK)==TYPE_TEXT) {  /* text data */
                printf("[T]:  \"");
                unsigned char *c=p+HDR;
                unsigned char l=0;
                while(l<e->len && ++l<=40) {
                    if(*c>=' ') printf("%c",*(c++)); else printf("_#%03d",*(c++));
                }
                if(l>=e->len) printf("\""); else printf(" ...");
            } else {    /* numeric data */
                printf("[N]:  %.10g",(double)rrec_dbl(e));
            }
            new_line();
        }
        p+=(HDR+((f_data_t *)p)->len);
    }
    new_line();
    printf("%u element",count);
    if(count!=1) printf("s");
    printf(", %lu bytes",total);
    new_line();
}


/*
[w] insp
inspect a specified data container or if no parameter is supplied - all data containers
*/
void fanf_insp(void) {
    unsigned int id=0;
    unsigned long total=0;
    f_data_t *e=get_data(stid,0);
    if(e) {
        char tn[NAMESPACE_SIZE] = {0};
        if(((e->id)&TYPE_MASK)!=TYPE_TEXT) longjmp(shell,ERROR_TYPE_MISMATCH);
        if((e->len+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
        if(e->len) memcpy(tn,((unsigned char *)e+HDR),e->len);
        set_data(stid,TYPE_MASK,0,0,-1);
        if(!(*tn)) return;
        if(*tn && tn[strlen(tn)]!=':') strcat(tn,":");  /* assert a lexically valid namespace */
        const unsigned char *c=find_word(tn);
        if(!c) longjmp(shell,ERROR_UNKNOWN_WORD);
        c+=(1+strlen(tn));
        id=(unsigned long)(c-Fmem)&(~TYPE_MASK);
    }
    unsigned int count=0;
    unsigned int x=0;
    unsigned char *p=pData;
    unsigned int oid=0;
    while((p-Fmem)<FMEM_SIZE) { /* check whether this element already exists in the chain */
        e=(f_data_t *)p;
        if(((e->id)&(~TYPE_MASK))!=oid) {
            oid=((e->id)&(~TYPE_MASK));
            x=0;
        }
        if(e->id!=SOURCE && e->id<STACK_DATA && (!id || oid==id)) {
            count++;
            printf("%4u ! ",x++);
            unsigned char *w=Fmem+oid-1;
            while(w && *(w-1)!=code[CODE_NEW]) w--; /* position the pointer to print the word name */
            if(w<Fmem || w>=(Fmem+FMEM_SIZE)) longjmp(shell,ERROR_INTERNAL_ERROR);
            while(*w!=':') printf("%c",*(w++));
            printf(" \t%10u  ",e->len);
            total+=(unsigned int)(e->len);
            if(((e->id)&TYPE_MASK)==TYPE_TEXT) {  /* text data */
                printf("[T]:  \"");
                unsigned char *c=p+HDR;
                unsigned char l=0;
                while(l<e->len && ++l<=40) {
                    if(*c>=' ') printf("%c",*(c++)); else printf("_#%03d",*(c++));
                }
                if(l>=e->len) printf("\""); else printf(" ...");
            } else {    /* numeric data */
                printf("[N]:  %.10g",(double)rrec_dbl(e));
            }
            new_line();
        }
        p+=(HDR+e->len);
    }
    new_line();
    printf("%u element",count);
    if(count!=1) printf("s");
    printf(", %lu bytes",total);
    new_line();
}


/*
handle insert and replace operations for source
input
(oprtype) 0:insert, 1:replace
*/
void ins_repl_src(char oprtype) {
    unsigned long x;
    for(x=0; x<MAX_PROCESS; x++) PC[x]=0;   /* immediately cancel all processes */

    fanf_mpf(pCode,MPF_CLEANUP);
    x=(unsigned long)get_num(1);
    if(x<0 || x>=(pCode-Fmem) || Fmem[x]!=code[CODE_NOP]) longjmp(shell,ERROR_INVALID_LOC);
    fanf_mpf(&Fmem[x+1],MPF_NAMESPACE);     /* find the namespace for this location */
    const unsigned char *c=&Fmem[x+1];
    long diff=0;
    char skipf;

    /* if the operation is 'replace' try to find the next %nop pseudocode */
    if(oprtype) {
        const unsigned char *co=c;
        while(c<pCode) {
            skipf=0;
            const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */
            if(a->code==code[CODE_NOP]) {
                c++;    /* skip it */
                break;
            }
            else if(a->code==find_atom("%text",0)->code) skipf=1;
            else if(a->code==find_atom("%skip",0)->code) skipf=1;
            if(a->len==-1) {    /* new word */
                c++;    /* skip the pseudocode */
                while(c<pCode && *c!=':') c++;  /* skip everything until : */
                if(c>=pCode) break; /* code has finished before the end of the word */
                c++;    /* skip the ending : */
            }
            else if(a->len==0) {    /* VLN */
                unsigned long v;
                c=get_vln((unsigned char *)(c+1),&v);
                if(skipf) c+=v;
            } else {    /* normal situation */
                c+=a->len;
            }
        }
        diff=(co-c);
    }

    /* move the rest of the code away for a new compilation and feed the parameter source */
    unsigned long len=pCode-c;
    unsigned char *away=pData-CMD_BUFFER_SIZE-len;
    if(away<pCode) longjmp(shell,ERROR_NO_MEMORY);
    memmove(away,c,len);        /* move the rest away so the new code can be safely complied in its place */
    pCode=&Fmem[x+1];
    set_data(SOURCE,TYPE_MASK,0,0,-1);
    f_data_t *e=get_data(stid,0);
    if(!e) longjmp(shell,ERROR_STACK_EMPTY);
    if(!PC[process] || !stid) e->id=SOURCE;
    flags|=FLAG_FORCE_CMPL; /* instruct f_source () not to execute anything but only to compile */
    f_source();

    /* start recompiling the relocated code */
    c=away;
    diff+=(pCode-&Fmem[x+1]);   /* calculate the difference in bytes between the old and the new beginning of the relocated code */
    const unsigned char *cend=(away+len);
    while(c<cend) {
        char amodf=0;
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */

        if(a->code==code[CODE_NEW]) {
            const unsigned char *w=(c+1);
            while(w<cend && *w!=':') w++;
            if(w>=cend) break;  /* code has finished before the end of the word */
            int wl=w-c;
            if(wl<2) longjmp(shell,ERROR_UNKNOWN_WORD); /* the word has invalid name */
            if((strlen(ns)+wl+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);   /* too long namespace */
            ns[strlen(ns)+wl]=0;
            memcpy(&ns[strlen(ns)],c+1,wl);
        }

        else if(a->code==code[CODE_EOW]) {
            if(*ns) {   /* remove the last word from the namespace */
                do {
                   ns[strlen(ns)-1]=0;
                } while(*ns && ns[strlen(ns)-1]!=':');
            }
        }

        else if(a->code==find_atom("%call",0)->code) amodf=1;
        else if(a->code==find_atom("%task",0)->code) amodf=1;
        else if(a->code==find_atom("%ref",0)->code) amodf=1;

        else if(a->code==find_atom("%text",0)->code) skipf=1;
        else if(a->code==find_atom("%skip",0)->code) skipf=1;

        /* jump to the next pseudocode (still on the current one now); refresh the pointers on the way */
        if(a->len==-1) {    /* new word */
            *(pCode++)=*(c++);  /* skip the pseudocode */
            while(c<cend && *c!=':') *(pCode++)=*(c++);         /* skip everything until : */
            if(c>=cend) break;  /* code has finished before the end of the word */
            *(pCode++)=*(c++);  /* skip the ending : */
        }
        else if(a->len==0) {    /* VLN */
            const unsigned char *co=(c+1);
            unsigned long v;
            c=get_vln((unsigned char *)co,&v);
            if(skipf) {
                add_atom((find_atom(0,a->code))->word);
                pCode_vln(v);
                memmove(pCode,c,v);
                pCode+=v;
                c+=v;
            }
            else if(amodf) {
                unsigned char *warn=0;
                char vlo=(c-co);
                if(v>=(unsigned long)(&Fmem[x+1]-Fmem)) v+=diff;
                if(Fmem[v-1]!=':') warn=pCode;
                add_atom((find_atom(0,a->code))->word);
                const unsigned char *po=pCode;
                pCode_vln(v);
                char vln=(pCode-po);
                diff+=(vln-vlo);
                if(warn) {
                    new_line();
                    printf("WARNING: [%04lx] invalid reference to address %04lx",(unsigned long)(warn-Fmem),v);
                }
            }
        } else {    /* normal situation */
            memmove(pCode,c,a->len);
            c+=a->len;
            pCode+=a->len;
        }

    }
    fanf_mpf(pCode,MPF_CLEANUP);
    flags|=FLAG_KEEP_PCODE;
}


/*
x s resrc
replace source line starting from index x (given by the 'list' word) with the new source s
*/
void fanf_resrc(void) {
    ins_repl_src(1);
}


/*
x s insrc
insert a new source line s starting from index x (given by the 'list' word)
*/
void fanf_insrc(void) {
    ins_repl_src(0);
}


/*
a l dump
hexadecimal dump of l bytes starting from address a (mainly for debug)
*/
void fanf_dump(void) {
    unsigned long l=(unsigned long)get_num(0);  /* get length */
    unsigned long a=(unsigned long)get_num(0);  /* get address */
    if(l<0 || a<0 || (a+l)>FMEM_SIZE) longjmp(shell,ERROR_BAD_PARAMETER);
    unsigned int addr=a&(~15);
    unsigned int len=(l&(~15))+16*(!!(l%16));
    while(len--) {
        if((addr%16)==0) {
            new_line();
            printf("%04x:  ",addr);
        }
        if(addr>=a && addr<(a+l)) printf("%02x",Fmem[addr]); else printf("..");
        printf("  ");
        addr++;
    }
    new_line();
}
