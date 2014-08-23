#include <conio.h>  /* WARNING! this library may not exist in all platforms; used only for the kbhit() function */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fanf.h"
#include "memory.h"
#include "atoms.h"
#include "shellw.h"

#ifdef MORE_WORDS_FILE
#include MORE_WORDS_FILE_INCLUDE(MORE_WORDS_FILE)
#endif

#define PROMPT_NORMAL   "| "        /* input prompt */
#define PROMPT_COMMENT  "|`"        /* input prompt when a multi-line commentary is being written */
#define TAB_SIZE 4                  /* number of characters inserted in the input string on [Tab] key */
#define CHR_TERM        " \r\n\t"   /* word terminating characters (plus NULL) */

/* list of words with pre-loaded codes (see code[] array and CODE_xxx constants) */
const char plwords[PRELOADED_CODES][11] = { "%nop", ";", "%new", "%nfmt", "if", "else", "endif", "do", "while", "loop", "repeat", "cont", "break" };

jmp_buf shell;              /* shell environment */
char ns[NAMESPACE_SIZE] = {0};  /* current namespace */
char nso[NAMESPACE_SIZE] = {0}; /* saved namespace (used to handle : references) */
char cmd[CMD_BUFFER_SIZE] = {0};    /* command buffer */
char *fsrc=cmd;             /* input source pointer for parsing and compilation; modified along with the compilation process */
int lines=0;                /* number of printed lines counter (actual only if the relevant function is used) */
int lines_before_pause=0;   /* printed lines before pause */
unsigned short flags=0;     /* FANF and FVM flags (see FLAG_xxx constants) */
unsigned char code[PRELOADED_CODES];    /* some important pre-loaded atomic codes */
unsigned char process=0;    /* current process */
unsigned char nested=0;     /* global nested calls counter */
unsigned int stid=0;        /* pre-calculated stack id for the current process (STACK_DATA+(process<<2)) */
unsigned char *PC[MAX_PROCESS] = {0};   /* process code pointers */
unsigned int newdc[MAX_PROCESS] = {0};  /* new/; depth counter for each process */
unsigned int refid[MAX_PROCESS] = {0};  /* ref id for each process during execution */
unsigned int refix[MAX_PROCESS] = {0};  /* ref index for each process during execution */
unsigned int refns[MAX_PROCESS] = {0};  /* refer namespace for each process during execution */
unsigned char ctldc[MAX_PROCESS] = {0}; /* depth counter for the execution control stack for each process */
void *files[MAX_FILES] = {0};           /* currently open files */
f_ctrl_t ctrl[MAX_PROCESS][MAX_CTRL];   /* execution control structures for each process */
unsigned char *taskPC=0;    /* new task PC */
unsigned char *runPC=0;     /* address of a word which needs to be executed - used by f_run() */
unsigned long addPC=0;      /* this value is added to the PC after the execution of VLN-encoded instructions */
unsigned char *opCode;      /* store the (pCode) value before execution */


/*
print boring stuff
*/
void f_author(const char *platform) {
    printf("FANF %s %s\r\n",VERSION,COPYRIGHT);
    if(platform && *platform) printf("Platform: %s\r\n",platform);
    printf("Total memory: %lu bytes, free: %lu bytes\r\n",(unsigned long)FMEM_SIZE,(unsigned long)(pData-pCode));
}


/*
print a new line and a message and wait the user to press [Enter] key if the predefined number of lines has been reached
*/
void new_line(void) {
    printf("\r\n");
    if(LINES_BEFORE_PAUSE && ++lines>=LINES_BEFORE_PAUSE) {
        printf("Press [Enter] to continue... ");
        while(kbhit()) getchar();
        lines=0;
        int c=0;
        do {
            c=getchar();
        } while(c!='\r' && c!='\n' && c!=3);
        if(c==3) longjmp(shell,ERROR_NONE); /* Ctrl-C: return to the shell */
        printf("\r\n");
        while(kbhit()) getchar();
    }
}


/*
initialise the FVM
*/
void fvm_init(void) {
    lines=0;
    nested=0;
    memset(PC,0,sizeof(PC));
    memset(newdc,0,sizeof(newdc));
    memset(refid,0,sizeof(refid));
    memset(refix,0,sizeof(refix));
    memset(refns,0,sizeof(refns));
    memset(ctrl,0,sizeof(ctrl));
    memset(ctldc,0,sizeof(ctldc));
    unsigned char t;
    for(t=0; t<MAX_FILES; t++) {
        if(files[t]) fclose(files[t]);
        files[t]=0;
    }
 }


/*
get a VLN-encoded number
input
(*address) address in Fmem[] where the VLN-encoded number starts
(*value) pointer to a buffer where the read number will be saved
output
*get_vln() address pointing after the VLN-encoded number
(*value) read VLN value
*/
unsigned char *get_vln(unsigned char *address, unsigned long *value) {
    *value=0;
    if(!address) return 0;
    if(*address)
        *value=*(address++);
    else
        if(*(++address)) {
            *value=*(address++);
            *value=((*value)<<8)+(*(address++));
        } else {
            address++;
            *value=*(address++);
            *value=((*value)<<8)+(*(address++));
            *value=((*value)<<8)+(*(address++));
            *value=((*value)<<8)+(*(address++));
        }
    return address;
}


/*
find a specified atom by its name or code
input
(*word) the word as text (terminated by NULL, Tab, CR, LF or space)
        if no word is supplied, the search is performed by code supplied in the second parameter (the function will handle errors itself in this case)
(*code) searched code (only used of (*word) is null or blank)
output
find_atom() pointer to the atom in the description table, or a null pointer if not found
*/
const f_atom_t *find_atom(const char *word, const unsigned char code) {
    if(!word || !(*word)) { /* if no word is supplied, find the atom by its code */
        const f_atom_t *a=atom;
        while(a->word)
            if(code!=a->code) a++; else break;
        if(!a) longjmp(shell,ERROR_UNKNOWN_CODE);
        return a;
    }
    const char *w=word;
    while(*w>' ') w++;
    int wl=w-word;  /* calculate the length of the input word until the terminating character */
    if(word[wl-1]==':') wl--;
    if(wl==0) return 0; /* the word is only a single : character */
    const f_atom_t *a=atom;
    while(a->word) {
        if(wl==strlen(a->word) && !memcmp(word,a->word,wl)) break;
        a++;
    }
    return(a->word? a : 0);
}


/*
add an atom to the compiled code
input
(*word) the atomic word as text
(*pCode) input compilation pointer
output
(*pCode) updated compilation pointer
*/
void add_atom(const char *word) {
    if(word && *word) {
        const f_atom_t *a=find_atom(word,0);
        if(!a) longjmp(shell,ERROR_INTERNAL_ERROR);
        *(pCode++)=a->code;
        if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
    }
}


/*
find the address of a specified word by its name
input
(*word) the word as text (the ending : character MUST be included)
output
find_word() pointer to the beginning of the word's definition in the FVM code, or a null pointer if not found
*/
const unsigned char *find_word(const char *word) {
    if(!word || !(*word)) return 0;
    char tn[NAMESPACE_SIZE] = {0};  /* namespace buffer */
    unsigned char *c=Fmem;
    char skipf;
    while(c<pCode) {
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);      /* find the atom by its code */
        if(a->code==code[CODE_NEW]) {  /* start of a new word detected */
            unsigned char *w=c+1;
            while(w<pCode && *w!=':') w++;
            if(w>=pCode) longjmp(shell,ERROR_UNEXPECTED_END);  /* code has finished before the end of the word */
            int wl=w-c;
            if(wl<2) longjmp(shell,ERROR_UNKNOWN_WORD);    /* the word has invalid name */
            if((strlen(tn)+wl+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);  /* too long namespace */
            tn[strlen(tn)+wl]=0;
            memcpy(&tn[strlen(tn)],c+1,wl);
            if(!strcmp(tn,word)) return c;  /* check is this our word? */
        }
        else if(a->code==code[CODE_EOW]) { /* end of a new word detected */
            if(!(*tn)) longjmp(shell,ERROR_NAMESPACE_OUT); /* expected never to happen because this situation is handled earlier during compilation */
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
            if(c>=pCode) longjmp(shell,ERROR_UNEXPECTED_END);  /* code has finished before the end of the word */
            c++;    /* skip the ending : */
        }
        else if(a->len==0) {    /* VLN */
            unsigned long v;
            c=get_vln(c+1,&v);
            if(skipf) c+=v;
        } else  /* normal situation */
            c+=a->len;
    }
    return 0;   /* not found */
}


/*
print a word by its call address
input
(*a) address supplied by %call, %task or %ref instruction
(*cns) optional: current namespace
*/
void print_addr_word(unsigned long a, const char *cns) {
    char tn[NAMESPACE_SIZE] = {0};
    unsigned char *addr=Fmem+a;
    unsigned char *c=Fmem;
    char skipf;
    while(c<addr) {
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */

        /* start of a new word detected */
        if(a->code==code[CODE_NEW]) {
            const unsigned char *w=c+1;
            while(w<addr && *w!=':') w++;
            int wl=w-c;
            if((strlen(tn)+wl+2)>=NAMESPACE_SIZE) { /* too long namespace */
                new_line();
                longjmp(shell,ERROR_NAME_TOO_LONG);
            }
            tn[strlen(tn)+wl]=0;
            memcpy(&tn[strlen(tn)],c+1,wl);
            if(++w==addr && *tn) {
                tn[strlen(tn)-1]=0;
                char *tns=tn;
                if(cns && !memcmp(tn,cns,strlen(cns))) tns=&tn[strlen(cns)];    /* remove the leading part if the word is in the current namespace */
                printf("%s",tns);
                return;
            }
        }

        /* end of a new word detected */
        else if(a->code==code[CODE_EOW]) {
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
    printf("???");
}


/*
create a binary string from a number
input
(num) the input number
output
bin() the output binary string
*/
char *bin(unsigned long num) {
    static char buf[65];
    *buf=0;
    while(num) {
        memmove(buf+1,buf,64);
        *buf='0'+(num&1);
        num>>=1;
    }
    return buf;
}


/*
determine the most suitable way for expressing a numeric constant and generate the pseudocode for it
input
(n) the numeric constant to be evaluated
(*pCode) position in the compiled source for the new pseudocode
output
(*pCode) new value of the pointer in the output code
*/
void pCode_number(const REAL n) {
    if(!pCode) return;
    unsigned long ni=(unsigned long)n;  /* used when storing integer constants */
    char rf=0;

    /* check is it integer */
    if((double)n==floor((double)n)) {

        /* 8-bit signed constant */
        if(n>=-128 && n<=127) {
            add_atom("%sint8");
            *(pCode++)=ni;
        }

        /* 8-bit unsigned constant */
        else if(n>=0 && n<=255) {
            add_atom("%uint8");
            *(pCode++)=ni;
        }

        /* 16-bit signed constant */
        else if(n>=-32768 && n<=32767) {
            add_atom("%sint16");
            *(pCode++)=(ni>>8);
            *(pCode++)=ni;
        }

        /* 16-bit unsigned constant */
        else if(n>=0 && n<=65535) {
            add_atom("%uint16");
            *(pCode++)=(ni>>8);
            *(pCode++)=ni;
        }

        /* 32-bit signed constant */
        else if(n>=((REAL)0-0x80000000) && n<=0x7fffffff) {
            add_atom("%sint32");
            *(pCode++)=(ni>>24);
            *(pCode++)=(ni>>16);
            *(pCode++)=(ni>>8);
            *(pCode++)=ni;
        }

        /* 32-bit unsigned constant */
        else if(n>=0 && n<=0xffffffff) {
            add_atom("%uint32");
            *(pCode++)=(ni>>24);
            *(pCode++)=(ni>>16);
            *(pCode++)=(ni>>8);
            *(pCode++)=ni;
        }

        /* otherwise present as a floating point number */
        else rf=1;

    } else
        rf=1;

    /* floating point number constant or large integer numbers */
    if(rf) {
        REAL nr=(REAL)n;
        add_atom("%real");
        memcpy(pCode,&nr,sizeof(REAL));
        pCode+=sizeof(REAL);
    }


    if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
}


/*
store a number in VLN notation
input
(n) the numeric constant to be evaluated
(*pCode) position in the compiled source for the new pseudocode
output
(*pCode) new value of the pointer in the output code
*/
void pCode_vln(const unsigned long n) {
    if(!pCode) return;

    /* one-byte values except for 0 */
    if(n && n<256) {
        *(pCode++)=n;
    }

    /* two-byte values */
    else if(n && n<65536) {
        *(pCode++)=0;
        *(pCode++)=(n>>8);
        *(pCode++)=n;
    }

    /* value 0 and four-byte values */
    else {
        *(pCode++)=0;
        *(pCode++)=0;
        *(pCode++)=(n>>24);
        *(pCode++)=(n>>16);
        *(pCode++)=(n>>8);
        *(pCode++)=n;
    }

    if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
}


/*
compile a single FANF word (word, constant or commentary)
input
(*fsrc) pointer to the input term to be parsed (terminated by NULL, Tab, CR, LF or space)
(*pCode) position in the compiled source for the new pseudocode
output
(*fsrc) new value of the pointer in the input source
(*pCode) new value of the pointer in the output code
*/
void fanf_compile_word(void) {
    if(!fsrc || *fsrc<=' ' || !pCode) {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
    }

    /* commentaries */
    if(*fsrc=='`' || (flags&FLAG_COMMENT)) {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
        add_atom("%skip");
        if(!(flags&FLAG_COMMENT)) fsrc++;   /* entering through the multi-line option - no need to skip anything */
        unsigned char *lp=pCode;    /* here will be inserted the length later */
        unsigned long len=0;
        memset(lp,0,6);
        pCode+=6;   /* make room for VLN (will be adjusted later) */
        if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
        if(*fsrc!='!' || (flags&FLAG_COMMENT)) {    /* long (possibly multi-line) comment */
            while(*fsrc && *fsrc!='`') {
                *(pCode++)=*(fsrc++);
                if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
            }
            if(*fsrc=='`') {    /* check if there is a closing ` character (maybe a multi-line comment?) */
                fsrc++; /* skip the ending ` */
                flags&=~FLAG_COMMENT;
                if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
            } else
                flags|=FLAG_COMMENT;
        } else      /* short comment */
            while(*fsrc && *fsrc!='\r' && *fsrc!='\n') {    /* note: only NULL, CR and LF are checked here (end of a line condition) */
                *(pCode++)=*(fsrc++); /* the leading ! character will be saved as well */
                if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
            }
        len=pCode-lp-6; /* calculate the length of the comment */
        pCode=lp;
        pCode_vln(len); /* store the length */
        if(pCode<(lp+6)) {  /* adjust the position of the data */
            memmove(pCode,lp+6,len);
            pCode+=len;
        }
    }

    /* text constants */
    else if(*fsrc=='"') {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
        add_atom("%text");
        fsrc++;
        unsigned char *lp=pCode;    /* here will be inserted the length later */
        unsigned long len=0;
        memset(lp,0,6);
        pCode+=6;   /* make room for VLN (will be adjusted later) */
        if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
        while(*fsrc>=' ') {
            if(*fsrc=='_') {    /* single characters preceded by _ */
                unsigned char c=0;
                fsrc++;
                if(*fsrc=='"' || *fsrc=='_') c=*fsrc;   /* " or _ character direct encoding */
                else if(*fsrc>='0') c=*fsrc-'0';        /* direct character code (ASCII-$30) */
                else if(*fsrc=='#') {   /* decimal character code */
                    char d=3;   /* exactly three decimal digits are required */
                    while(d--) {
                        fsrc++;
                        if(*fsrc && strchr("0123456789",*fsrc))
                            c=(10*c)+(*fsrc-'0');
                        else
                            longjmp(shell,ERROR_BAD_SYNTAX);
                    }
                }
                else if(*fsrc=='$') {   /* hexadecimal character code */
                    char d=2;   /* exactly two hexadecimal digits are required */
                    while(d--) {
                        fsrc++;
                        if(*fsrc && strchr("0123456789",*fsrc)) c=(c<<4)+(*fsrc-'0');
                        else if(*fsrc && strchr("ABCDEF",*fsrc)) c=(c<<4)+10+(*fsrc-'A');
                        else if(*fsrc && strchr("abcdef",*fsrc)) c=(c<<4)+10+(*fsrc-'a');
                        else longjmp(shell,ERROR_BAD_SYNTAX);
                    }
                }
                else if(*fsrc=='%') {   /* binary character code */
                    char d=8;   /* exactly eight binary digits are required */
                    while(d--) {
                        fsrc++;
                        if(*fsrc=='0') c<<=1;
                        else if(*fsrc=='1') c=(c<<1)+1;
                        else longjmp(shell,ERROR_BAD_SYNTAX);
                    }
                }
                else {  /* invalid sequence */
                    longjmp(shell,ERROR_BAD_SYNTAX);
                }
                fsrc++;
                *(pCode++)=c;
                if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
            } else
                if(*fsrc!='"') {
                    *(pCode++)=*(fsrc++);
                    if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
                } else {
                    len=pCode-lp-6; /* calculate the length of the text constant */
                    break;
                }
        }
        if(*fsrc!='"') longjmp(shell,ERROR_BAD_SYNTAX);
        fsrc++; /* skip the ending " */
        pCode=lp;
        pCode_vln(len); /* store the length */
        if(pCode<(lp+6)) {  /* adjust the position of the data */
            memmove(pCode,lp+6,len);
            pCode+=len;
        }
    }

    /* decimal constants (integer and floating point) */
    else if(*fsrc && (strchr("0123456789.#",*fsrc) || (*fsrc=='-' && strchr("0123456789.",*(fsrc+1))))) {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
        REAL v=0; /* value */
        REAL r=0.1;   /* magnitude for fractions */
        REAL p=0; /* scientific exponent */
        char dpf=0; /* decimal point flag */
        char epf=0; /* 'E' pointer flag */
        char sign=1;    /* decimal sign */
        char esign=1;   /* E-sign */
        if(*fsrc=='#') fsrc++;  /* only for compatibility with the initial documentation */
        else if(*fsrc=='-') {   /* negative number */
            fsrc++;
            sign=-1;
        }
        for(; *fsrc>' '; fsrc++) {
            if(*fsrc && strchr("0123456789",*fsrc)) {
                if(!epf) {  /* normal representation */
                    if(!dpf)
                        v=(10*v)+(*fsrc-'0');
                    else {
                        v+=r*(*fsrc-'0');
                        r/=10;
                    }
                } else      /* scientific representation */
                    p=(10*p)+(*fsrc-'0');
            }
            else if(*fsrc=='.') {
                if(dpf || epf) longjmp(shell,ERROR_BAD_SYNTAX);    /* only one decimal point is allowed and it must be before any 'E' */
                dpf=1;
            }
            else if(*fsrc=='E' || *fsrc=='e') {
                if(epf || v==0) longjmp(shell,ERROR_BAD_SYNTAX);   /* only one 'E' is allowed and it must follow a non-zero number */
                epf=1;
                if(*(fsrc+1)=='-') {    /* check for a '-' character immediately following the 'E' */
                    fsrc++;
                    esign=-1;
                }
            }
            else break;
        }
        if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
        p=pow(10,p);
        if(esign<0) p=1/p;
        pCode_number((REAL)(sign*v*p));
        return;
    }

    /* hexadecimal constants */
    else if(*fsrc=='$') {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
        unsigned long v=0;
        for(fsrc++; *fsrc>' '; fsrc++) {
            if(*fsrc && strchr("0123456789",*fsrc)) v=(v<<4)+(*fsrc-'0');
            else if(*fsrc && strchr("ABCDEF",*fsrc)) v=(v<<4)+10+(*fsrc-'A');
            else if(*fsrc && strchr("abcdef",*fsrc)) v=(v<<4)+10+(*fsrc-'a');
            else break;
        }
        if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
        add_atom("%nfmt");
        *(pCode++)=0;   /* hexadecimal type */
        if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
        pCode_number((REAL)v);
        return;
    }

    /* binary constants */
    else if(*fsrc=='%') {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
        unsigned long v=0;
        for(fsrc++; ; fsrc++) {
            if(*fsrc=='0') v<<=1;
            else if(*fsrc=='1') v=(v<<1)+1;
            else break;
        }
        if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
        add_atom("%nfmt");
        *(pCode++)=1;   /* binary type */
        if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
        pCode_number((REAL)v);
        return;
    }

    /* references */
    else if(*fsrc=='@') {
        if((flags&FLAG_COLON) || (flags&FLAG_REFER)) {
            longjmp(shell,ERROR_BAD_SYNTAX);
        } else {
            fsrc++; /* skip the @ character (only!) */
            flags|=FLAG_REFER;
        }
        return;
    }

    /* namespaces */
    else if(*fsrc==':') {
        if(!(*ns)) longjmp(shell,ERROR_NAMESPACE_OUT); /* refering to something out of any namespace */
        if(!(flags&FLAG_COLON)) strcpy(nso,ns);
        fsrc++; /* skip the : character (only!) */
        do {    /* remove the last word from the namespace */
            ns[strlen(ns)-1]=0;
        } while(*ns && ns[strlen(ns)-1]!=':');
        flags|=FLAG_COLON;
        return;
    }

    /* words and atoms */
    else if(*fsrc && !strchr(CHR_TERM,*fsrc)) {

        /* check is it an atomic word */
        const f_atom_t *a=find_atom(fsrc,0);
        if(a) {
            if((flags&FLAG_COLON) || (flags&FLAG_REFER)) longjmp(shell,ERROR_BAD_SYNTAX);
            fsrc+=strlen(a->word);
            if(*fsrc==':') longjmp(shell,ERROR_REDEF_ATOM);
            if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
            fsrc++; /* skip the terminating character */
            if(!strcmp(a->word,";")) {  /* check for end of a definition */
                if(!(*ns)) longjmp(shell,ERROR_UNEXPECTED_END);    /* ; without a relevant beginning */
                do {    /* remove the last word from the namespace */
                    ns[strlen(ns)-1]=0;
                } while(*ns && ns[strlen(ns)-1]!=':');
                flags|=FLAG_WAS_EOW;
            }
            *(pCode++)=a->code;   /* atomic words */
            if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
            return;
        }

        /* add the word to the namespace */
        char taskf=0;
        if(*fsrc=='~') {
            fsrc++;
            taskf=1;    /* raise the multitasking flag */
        }
        char *w=fsrc;
        while(*w>' ') w++;
        int wl=w-fsrc;  /* (wl) will be at least 1 as blank words are handled in the beginning of the function) */
        if((strlen(ns)+wl+2)>=NAMESPACE_SIZE) longjmp(shell,ERROR_NAME_TOO_LONG);
        ns[strlen(ns)+wl]=0;
        memcpy(&ns[strlen(ns)],fsrc,wl);

        /* new word definition */
        if(fsrc[wl-1]==':') {
            if((flags&FLAG_REFER) || ((flags&FLAG_COLON) && !(flags&FLAG_REFER)) || taskf) longjmp(shell,ERROR_BAD_SYNTAX);
            if(find_word(ns)) longjmp(shell,ERROR_REDEF_WORD); /* this word is already defined */
            if(pCode==Fmem) add_atom("%nop");   /* add a blank operation if the new word is starting from address 0 */
            add_atom("%new");
            w=fsrc;
            int wlc=wl-1;
            while(wlc--) {  /* copy the word name into the code */
                if(*w && !strchr("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_",*w)) longjmp(shell,ERROR_BAD_SYNTAX);
                *(pCode++)=*(w++);
                if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
            }
            if(*w!=':') longjmp(shell,ERROR_BAD_SYNTAX);
            *(pCode++)=*(w++);  /* copy the : character as well */
            if(pCode>=pData) longjmp(shell,ERROR_CODE_TOO_LONG);
        }

        /* use or refer to a word */
        else {
            if((flags&FLAG_REFER) && taskf) longjmp(shell,ERROR_BAD_SYNTAX);
            if(*ns && ns[strlen(ns)]!=':') {    /* assert a lexically valid namespace */
                strcat(ns,":");
                wl++;
            }
            const unsigned char *wpa=find_word(ns); /* try the local namespace first */
            if(!wpa) wpa=find_word(&ns[strlen(ns)-wl]); /* if not there - try the global namespace */
            if(!wpa) longjmp(shell,ERROR_UNKNOWN_WORD);
            if(ns[strlen(ns)-1]==':') wl--;
            if(flags&FLAG_REFER) {
                add_atom("%ref");
            } else {
                if(taskf) add_atom("%task"); else add_atom("%call");
            }
            while(wpa<pCode && *wpa!=':') wpa++;
            wpa++;  /* skip the : as well */
            if(wpa>=pCode) longjmp(shell,ERROR_UNEXPECTED_END);
            pCode_vln(wpa-Fmem);
            if(*ns) {   /* remove the word from the namespace */
                do {
                    ns[strlen(ns)-1]=0;
                } while(*ns && ns[strlen(ns)-1]!=':');
            }

        }

        /* move to the next word */
        fsrc+=wl;
        if(*fsrc && !strchr(CHR_TERM,*fsrc)) longjmp(shell,ERROR_BAD_SYNTAX);
        fsrc++; /* skip the terminating character */
        if(flags&FLAG_COLON) strcpy(ns,nso);
        flags&=~(FLAG_COLON|FLAG_REFER);
    }
}


/*
execute FANF pseudocode starting from a given address
input
(*pc) entry point for execution
(newpf) 'new process' flag: if raised, a new process is created, otherwise the current process' PC is assigned with the entry address
        see NEWPF_xxx constants
*/
void fanf_execute(const unsigned char *pc, char newpf) {

    /* find a new process slot, make a copy of the current stack for it and load its PC */
    if(newpf) {
        int p;
        for(p=0; p<MAX_PROCESS && PC[p]; p++);
        if(p>=MAX_PROCESS) longjmp(shell,ERROR_MAX_PROCESSES); /* no free process slot */
        if(PC[process]) {
            stid=(STACK_DATA+(process<<2));
            unsigned int newid=(STACK_DATA+(p<<2));
            f_data_t *e=(f_data_t *)pData;
            do {    /* delete any pre-existing stack elements with id's of the current process */
                e=get_data(newid,0);
                if(e) set_data(newid,TYPE_MASK,0,0,-1);
            } while(e);
            unsigned int v=0;
            e=(f_data_t *)pData;
            do {    /* count all stack elements for the current process */
                e=get_data(stid,v);
                if(e) {
                    v++;
                    e=(f_data_t *)(((unsigned char *)e+HDR)+e->len);
                }
            } while(e);
            while(v) {
                e=get_data(stid,--v);
                set_data(newid,((e->id)&TYPE_MASK),-1,((unsigned char *)e+HDR),e->len);
            }
        }
        process=p;
    }
    PC[process]=(unsigned char *)pc;
    refns[process]=(unsigned int)(pc-Fmem);

    /* execution loop */
    runPC=0;
    taskPC=0;
    char xf=0;
    for(;;) {
        if(kbhit() && getchar()==3) longjmp(shell,ERROR_NONE);  /* Ctrl-C: return to the shell */
        if(PC[process]>=pCode) PC[process]=0;   /* end of code reached */
        if(PC[process]) {
            xf=1;
            stid=(STACK_DATA+(process<<2));
            addPC=0;
            const f_atom_t *a=find_atom(0,*PC[process]);
            if(a->type==W_SHELL && nested) longjmp(shell,ERROR_SHELL_WORD);
            char exec=!!(a->func);
            if(!memchr(code,a->code,PRELOADED_CODES)) { /* the 'chosen ones' are executed unconditionally */
                if(newdc[process] && a->code!=code[CODE_EOW] && a->code!=code[CODE_NEW]) exec=0;    /* words definitions are skipped during execution */
                if(ctldc[process]) {
                    f_ctrl_t *c=&ctrl[process][ctldc[process]-1];
                    if(c->type==BLANK || (c->type==IFEND && !(c->condition)) || c->contf) exec=0;
                }
            }
            if(exec) (a->func)();
            if(!PC[process]) return;    /* if the runtime function has cleared the PC, that means it is time to exit now */
            if(a->len==-1) {        /* new word */
                PC[process]++;      /* skip the pseudocode */
                while(PC[process]<pCode && *PC[process]!=':') PC[process]++;    /* skip everything until : */
                if(PC[process]>=pCode) longjmp(shell,ERROR_UNEXPECTED_END);     /* code has finished before the end of the word */
                PC[process]++;      /* skip the ending : */
            }
            else if(a->len==0) {    /* VLN */
                unsigned long l;
                unsigned char *p=get_vln(PC[process]+1,&l);
                PC[process]=p+addPC;
            } else  /* normal situation */
                PC[process]+=a->len;
            if(taskPC) {
                if(++nested>=MAX_NESTED) longjmp(shell,ERROR_NESTING);
                fanf_execute(taskPC,NEWPF_NEW);
                if(nested) nested--;
            }
            if(runPC) {
                PC[process]=runPC;
                runPC=0;
            }
        }
        if(++process>=MAX_PROCESS) process=0;
        if(!process) {  /* check if any process is still active */
            if(!xf) break;
            xf=0;
        }
    }

}


/*
multi-purpose function; perforing the oprtation selected by the value in (opr)
input
(*cpos) position whose namespace needs to be determined or (pCode) for cleanup
(opr) select operation to perform (see MPF_xxx constants)
output
(ns[]) actual namespace (only if cleanup operation has been selected)
*/
void fanf_mpf(const unsigned char *cpos, char opr) {
    unsigned char *c=Fmem;
    unsigned char *p=pCode;
    char tn[NAMESPACE_SIZE] = {0};  /* namespace buffer */
    char skipf;
    while(c<cpos) {
        skipf=0;
        const f_atom_t *a=find_atom(0,*c);  /* find the atom by its code */

        if(a->code==code[CODE_NEW]) {
            const unsigned char *w=(c+1);
            while(w<pCode && *w!=':') w++;
            int wl=w-c;
            if(w>=pCode || wl<2 || (strlen(tn)+wl+2)>=NAMESPACE_SIZE) break;
            tn[strlen(tn)+wl]=0;
            memcpy(&tn[strlen(tn)],c+1,wl);
        }

        else if(a->code==code[CODE_EOW]) {
            if(*tn) {   /* remove the last word from the namespace */
                do {
                   tn[strlen(tn)-1]=0;
                } while(*tn && tn[strlen(tn)-1]!=':');
            }
            if(!(*tn)) p=(c+a->len);
        }

        else if(a->code==find_atom("%text",0)->code) skipf=1;
        else if(a->code==find_atom("%skip",0)->code) skipf=1;

        /* jump to the next pseudocode (still on the current one now) */
        if(a->len==-1) {    /* new word */
            c++;    /* skip the pseudocode */
            while(c<pCode && *c!=':') c++;  /* skip everything until : */
            if(c>=pCode) return;    /* previously: longjmp(shell,ERROR_UNEXPECTED_END); */
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
    if(opr==MPF_NAMESPACE) {
        strcpy(ns,tn);
    }
    else if(opr==MPF_CLEANUP) {
        pCode=p;
        if(pCode>Fmem && *(pCode-1)!=code[CODE_NOP]) add_atom("%nop");  /* mark the end of this source line */
    }
}


/*
text line editor
input
(*buf) text buffer
(bufsz) maximum size of the text buffer including the terminating 0 character
(x) initial position of the cursor (starting from 0)
output
(*buf) text after editing
*/
void line_edit(char *buf, unsigned int bufsz, int x) {
    if(!buf) return;
    if(!bufsz) {
        *buf=0;
        return;
    }
    buf[bufsz-1]=0;
#ifndef USE_FGETS
    if(x<0) x=0;
    if(x>strlen(buf)) x=strlen(buf);
    printf("%s",buf);
    int t=strlen(buf);
    while(t-->x) printf("%c",KEY_BACKSPC);
    for(;;) {
        unsigned long ch=getchar();
        if(!ch || ch==KEY_ESC) ch=(ch<<8)+getchar();    /* a two-byte extended keyboard code has been received */
        if(ch>=(KEY_ESC<<8)) ch=(ch<<8)+getchar();      /* a three-byte escape sequence has been received */
        if((ch>=' ' && ch<0x100) || ch==KEY_TAB) {      /* characters and [Tab] key */
            t=1;
            if(ch==KEY_TAB) {   /* [Tab] key will produce (TAB_SIZE) number of space characters */
                ch=' ';
                t=TAB_SIZE;
            }
            while(t--) {
                if(strlen(buf)<(bufsz-1)) {
                    buf[x+1]=0;
                    buf[x++]=(char)ch;
                    printf("%c",(char)ch);
                } else
                    printf("\a");   /* alarming when the buffer limit has been reached */
            }
        }
        else if(
#ifdef USE_EXTENDED_KEYS
        ch==KEY_DEL ||
#endif  /* USE_EXTENDED_KEYS */
        ch==KEY_BACKSPC) {      /* backspace or [Del] key */
            if(
#ifdef USE_EXTENDED_KEYS
            ch==KEY_DEL ||
#endif  /* USE_EXTENDED_KEYS */
            (ch==KEY_BACKSPC && x)) {
                if(ch==KEY_BACKSPC) {
                    printf("%c",KEY_BACKSPC);
                    x--;
                }
                memmove(&buf[x],&buf[x+1],strlen(buf)-x+1);
                printf("%s %c",&buf[x],KEY_BACKSPC);
                t=strlen(buf);
                while(t-->x) printf("%c",KEY_BACKSPC);
            }
        }
        else if(ch==KEY_ENTER || ch=='\n') { /* [Enter] key */
            /* clear all characters after the current position */
            /* while(x<bufsz) {
                if(buf[x]) {
                    buf[x]=0;
                    printf(" ");
                }
                x++;
            } */
            /* printf("%c\n",KEY_ENTER); */
            break;
        }
#ifdef USE_EXTENDED_KEYS
        else if(ch==KEY_LEFT) {     /* left arrow */
            if(x) {
                printf("%c",KEY_BACKSPC);
                x--;
            }
        }
        else if(ch==KEY_RIGHT) {    /* right arrow */
            if(x<strlen(buf)) printf("%c",buf[x++]);
        }
        else if(ch==KEY_HOME) {     /* [Home] key will move the cursor to the beginning of the line */
            t=x;
            while(t--) printf("%c",KEY_BACKSPC);
            x=0;
        }
        else if(ch==KEY_END) {      /* [End] key will move the cursor to the end of the line */
            t=x;
            while(t--) printf("%c",KEY_BACKSPC);
            printf("%s",buf);
            x=strlen(buf);
        }
#endif  /* USE_EXTENDED_KEYS */
    }
#else   /* USE_FGETS */
    fgets(buf,bufsz-1,stdin);   /* this is the alternative way of doing the user input; a bit limited but sometimes might be of help */
    if(*buf && buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]=0;  /* remove the closing LF from the input string */
#endif  /* USE_FGETS */
}


/*
try to detect the amount of system free memory
output
get_freeram() the detected allocatable amount of RAM
*/
unsigned long get_freeram(void) {
    size_t size=(unsigned long)-1;  /* tested size for allocation */
    size_t umin=size;   /* unsuccessful minimum */
    size_t smax=0;      /* successful maximum */
    do {
        unsigned char *p=(unsigned char *)malloc(size);
        if(p) {
            free(p);
            if(size==umin) break;
            if(size>smax) smax=size;
            size+=(size/2);
            if(size>=umin) size=umin-(umin-smax)/2;
        } else {
            if(size<umin) umin=size;
            size/=2;
            if(size<=smax) size=smax+(umin-smax)/2;
        }
    } while((umin-smax)>=256);  /* the number here defines the 'granularity' of the tested memory */
    return (unsigned long)smax;
}


/*
print an error message and return to the shell
input
(erc) error code (see ERROR_xxx constants)
*/
void fanf_error(int erc) {
    static char fanf_error_executing=0;
    if(erc<=ERROR_NONE) return;
    if(!fanf_error_executing) { /* using this to prevent recursive entering in this block */
        fanf_error_executing=1;
        const unsigned char *ew=find_word("ERROR:");    /* try to find the error handler word */
        if(ew) {
            while(ew<pCode && *ew!=':') ew++;
            ew++;   /* skip the : as well */
            if(ew>=pCode) longjmp(shell,ERROR_UNEXPECTED_END);
            set_num(stid,TYPE_SINT,-1,(REAL)erc);       /* provide the error code to the handler */
            fanf_execute(ew,NEWPF_CURRENT);
        }
        fanf_error_executing=0;
    }
    printf("\r\n");
    if(flags&FLAG_COMPILE) {
        while(*(fsrc-1) && !strchr(CHR_TERM,*(fsrc-1))) fsrc--;
        while(*fsrc && !strchr(CHR_TERM,*fsrc)) printf("%c",*(fsrc++));
        printf("\r\nCOMPILATION");
    } else {
        if(PC[process]) {
            printf("in word [");
            print_addr_word(refns[process],0);
            printf("], address %02lx",(unsigned long)(PC[process]-Fmem));
        }
        printf("\r\nEXECUTION");
    }
    printf(" ERROR %u: ",erc);
    switch(erc) {
        default: printf("(no information)"); break;
        /* case ERROR_NONE: break; */ /* no error message if everything is okay */
        case ERROR_BAD_SYNTAX: printf("bad syntax"); break;
        case ERROR_UNKNOWN_WORD: printf("unknown word"); break;
        case ERROR_INTERNAL_ERROR: printf("internal error"); break;
        case ERROR_REDEF_ATOM: printf("redefining atomic word"); break;
        case ERROR_REDEF_WORD: printf("the word is already defined"); break;
        case ERROR_CODE_TOO_LONG: printf("compiled code is too long"); break;
        case ERROR_NAME_TOO_LONG: printf("too long name"); break;
        case ERROR_UNEXPECTED_END: printf("unexpected end"); break;
        case ERROR_NAMESPACE_OUT: printf("out of any namespace"); break;
        case ERROR_NO_MEMORY: printf("out of memory"); break;
        case ERROR_UNKNOWN_CODE: printf("undefined code"); break;
        case ERROR_SHELL_WORD: printf("shell word used in code"); break;
        case ERROR_STACK_EMPTY: printf("the stack is empty"); break;
        case ERROR_MAX_PROCESSES: printf("no free process slot"); break;
        case ERROR_MEMORY_REF: printf("invalid memory address"); break;
        case ERROR_NESTING: printf("maximum nesting levels reached"); break;
        case ERROR_TYPE_MISMATCH: printf("data type mismatch"); break;
        case ERROR_NO_REFERENCE: printf("missing data container reference"); break;
        case ERROR_DIV_ZERO: printf("division by zero"); break;
        case ERROR_INVALID_MATH: printf("invalid mathematical operation"); break;
        case ERROR_UNDEFINED_VAR: printf("undefined data container"); break;
        case ERROR_INVALID_INDEX: printf("invalid index"); break;
        case ERROR_ATOM_LISTING: printf("atomic words cannot be listed"); break;
        case ERROR_NESTED_CTRL: printf("maximum nested branches"); break;
        case ERROR_UNEXP_BREAK: printf("unexpected 'break'"); break;
        case ERROR_UNEXP_CONT: printf("unexpected 'continue'"); break;
        case ERROR_UNEXP_ELSE: printf("unexpected 'else'"); break;
        case ERROR_UNEXP_ENDIF: printf("unexpected 'endif'"); break;
        case ERROR_UNEXP_REPEAT: printf("unexpected 'repeat'"); break;
        case ERROR_INVALID_LOC: printf("invalid memory location"); break;
        case ERROR_INVALID_FMT: printf("invalid format specifier"); break;
        case ERROR_MANY_FILES: printf("unable to assign a file handler"); break;
        case ERROR_UNKNOWN_FILE: printf("unknown file handler"); break;
        case ERROR_STORAGE: printf("unable to store the library"); break;
        case ERROR_BAD_PARAMETER: printf("parameter out of range"); break;

#ifdef MORE_WORDS_FILE
#define MORE_WORDS_ERR
#include MORE_WORDS_FILE_INCLUDE(MORE_WORDS_FILE)
#undef MORE_WORDS_ERR
#endif

    }
    fvm_init();
    set_data(SOURCE,TYPE_MASK,0,0,-1);
    pCode=opCode;
    flags&=(FLAG_COMMENT | FLAG_RESTR_OK);  /* clear all flags except these */
}


/*
main FANF shell entry function
input
(*platform) optional string with the platform name
(memsize) desired FANF memory size in number of kilobytes; if 0 the shell will try to allocate the maximum available
*/
void FANF(const char *platform, unsigned long memsize) {

    ASSERT(sizeof(REAL)>4); /* FANF requires that the size of a floating point number must be greater than 32 bits */
    lines_before_pause=LINES_BEFORE_PAUSE;
    if(memsize) FMEM_SIZE=(memsize<<10); else FMEM_SIZE=get_freeram();
    Fmem=(unsigned char *)malloc(FMEM_SIZE);    /* this memory will never be freed under normal conditions */
    unsigned char c=0;
    while(c<PRELOADED_CODES) {
        const f_atom_t *a=find_atom(plwords[c],0);
        if(!Fmem || !a) {
            printf("\r\n\n\nFANF: FATAL ERROR (sorry!)\r\n\n\n");
            if(Fmem) free(Fmem);
            return;
        }
        code[c++]=a->code;
    }
    pCode=&Fmem[0];
    pData=&Fmem[FMEM_SIZE];
    fanf_restore(); /* restore saved library */
    f_author(platform);
    new_line();
    fvm_init();
    f_run();

    /* main loop starts here */
    fanf_error(setjmp(shell));
    if(runPC) {
        flags&=~FLAG_DIRECT;
        fanf_execute(runPC,NEWPF_CURRENT);
    }

    /* take the user input */
    fanf_mpf(pCode,MPF_NAMESPACE);  /* actualise the namespace */
    printf("\r\n%s",ns);            /* print the current namespace as part of the prompt */
    if(flags&FLAG_COMMENT) printf("%s",PROMPT_COMMENT); else printf("%s",PROMPT_NORMAL);    /* print the prompt */
    memset(cmd,0,CMD_BUFFER_SIZE);
    while(kbhit()) getchar();
    clearerr(stdin);
    line_edit(cmd,CMD_BUFFER_SIZE,0);   /* previously: fgets(cmd,CMD_BUFFER_SIZE-1,stdin); */
    if(strlen(cmd)>=CMD_BUFFER_SIZE-2) {
        while(*cmd && !strchr(CHR_TERM,cmd[strlen(cmd)-1])) cmd[strlen(cmd)-1]=0;
        while(*cmd && strchr(CHR_TERM,cmd[strlen(cmd)-1])) cmd[strlen(cmd)-1]=0;
        printf("WARNING: excessive characters have been trimmed. Processing following input:\r\n%s\r\n",cmd);
    }

    /* perform 'hard reset' without going into compilation */
    if(!strcmp(cmd,"__CLEAR__")) {
        flags=0;
        pCode=&Fmem[0];
        pData=&Fmem[FMEM_SIZE];
        memset(ns,0,NAMESPACE_SIZE);
        *cmd=0;
    }

    /* do the job */
    stid=0;
    lines=0;
    flags&=~(FLAG_COMPILE | FLAG_KEEP_PCODE | FLAG_FORCE_CMPL | FLAG_DIRECT);
    if(*cmd) {
        set_data(SOURCE,TYPE_TEXT,-1,cmd,strlen(cmd));
        opCode=pCode;
        f_source();
    }

    longjmp(shell,ERROR_NONE);
}
