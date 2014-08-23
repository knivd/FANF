#ifndef ATOMS_H
#define ATOMS_H

#include "memory.h"

/* definition of the 'REAL' type of numbers in FANF */
/* it is assumed that (double) is more than 4 bytes long; in some compilers this will have to be replaced by (long double) */
#ifndef REAL
#define REAL double
#endif

/* word types */
#define W_ATOM  0       /* normal atomic word */
#define W_SHELL 1       /* shell word (only executed from command line) */
#define W_EXTRA 2       /* platform extra word; not part of the general FANF syntax, but supplied for the particular hardware platform */

/* definition of built-in FANF atomic words */
typedef struct {
    char *word;         /* ASCIIZ text representation */
    unsigned char type; /* word type (see W_xxx constants) */
    unsigned short code; /* code */
    signed char len;    /* length in number of bytes in the compiled code; there are a few 'special' cases: */
                        /* 0: pseudocode plus following VLN */
                        /* -1: pseudocode plus following characters until : (inclusive) */
    void (*func)(void); /* executing function (used in runtime) */
                        /* if this is set to NULL, the atom will not have a runtime executed function */
} f_atom_t;

/* built-in FANF atomic words; a FANF program is compiled to FVM-code consisting of these words only */
extern const f_atom_t atom[];

/*
read a number from a data record (from the data memory)
input
(*e) data record
output
read_dbl() the number read
*/
REAL rrec_dbl(f_data_t *e);

/*
get the top number from the stack; will generate an error if the element contains text data
NOTE: the function will remove the top element from the stack
input
(osc) 'occurence skip counter' passed directly to the get_data() function; selects which in line element to get as 0 defines the top
output
get_num(0) value from the stack
*/
REAL get_num(unsigned int osc);

/*
automatically determine the best size of a number and push it into the stack
input
(id) stack record id
(type) data type (see TYPE_xxx constants)
(osc) 'occurence skip counter' passed directly to the set_data() function
(n) the value to be pushed
*/
void set_num(unsigned int id, char type, int osc, REAL n);

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
f_data_t *get_text(int osc, unsigned char **data);

/*
compile (and execute) FANF source code
*/
void f_source(void);

/*
run the last top level word
*/
void f_run(void);

/*
store the library into NVM
(external)
*/
void fanf_store(void);

/*
restore the library from NVM
(external)
*/
void fanf_restore(void);

#endif
