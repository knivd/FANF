#ifndef FANF_H
#define FANF_H

#define DEBUG

/*
=====================================================================================================================
FVM uses recursive style execution of the functions, and that requires a bit more processor stack. The safe processor
stack size (for the C-compiler) can be roughly estimated at (128+(2*NAMESPACE_SIZE)+(16*MAX_PROCESS)) bytes
=====================================================================================================================

preprocessor options for porting to various platforms:

PLATFORM "platform_name"        name of the current hardware platform as a string
MORE_WORDS_FILE "filename"      platform-dependent atomic words and functions (using MORE_WORDS_DEF preprocessor definitions to distinguish the sections)

*/

/*
helper function; for more helper functions see 'atoms.c'
*/
extern void __attribute__ ((weak)) platform_init(void); /* initialise hardware platform */

/* ========================================================================================================================= */
/* MAIN FANF PARAMETERS */

#define VERSION "r1408"

#ifndef LINES_BEFORE_PAUSE
#define LINES_BEFORE_PAUSE (23) /* lines before the output is paused and the user asked for a [Enter] key; used by some of the direct mode words */
#endif                          /* value 0 here will disable this functionality */

#ifndef NAMESPACE_SIZE
#define NAMESPACE_SIZE  (256)   /* maximum length of a word together with all of its parent words (namespace) */
#endif

#ifndef MAX_PROCESS
#define MAX_PROCESS (21)        /* number of parallel processes supported in FVM (maximum 256) */
#endif

#ifndef MAX_NESTED
#define MAX_NESTED (40)         /* maximum depth of nested calls (maximum 255) */
#endif

#ifndef MAX_CTRL
#define MAX_CTRL (32)           /* maximum depth of nested control structures (maximum 255) */
#endif

#ifndef MAX_FILES
#define MAX_FILES (7)           /* maximum number of simultaneously open file handlers in the system (maximum 255) */
#endif

/*
other possible #define statements to be used in project building preprocessor options if needed (but not recommended!)
DISABLE_HELP    disable the inclusion of the basic help information (saves about 1-2k of program code)
USE_FGETS       the line input function will use standard fgets() function
*/

/* ========================================================================================================================= */

#ifndef USE_EXTENDED_KEYS
#define USE_EXTENDED_KEYS   /* this enables the use of extended keys for editing (only if USE_FGETS is not enabled as well, otherwise this option is ignored) */
#endif

/* basic VT100 keyboard codes */
#define KEY_ESC         0x1b
#define KEY_ENTER       '\r'
#define KEY_BACKSPC     '\b'
#define KEY_TAB         '\t'
#define KEY_DEL         0x7f
#define KEY_LEFT        ((KEY_ESC<<16)+('['<<8)+'D')
#define KEY_RIGHT       ((KEY_ESC<<16)+('['<<8)+'C')
#define KEY_UP          ((KEY_ESC<<16)+('['<<8)+'A')
#define KEY_DOWN        ((KEY_ESC<<16)+('['<<8)+'B')
#define KEY_HOME        ((KEY_ESC<<16)+('['<<8)+'H')
#define KEY_END         ((KEY_ESC<<16)+('['<<8)+'K')

/* ========================================================================================================================= */

#define COPYRIGHT "(C) Konstantin Dimitrov [knivd@me.com]"  /* PLEASE DO NOT CHANGE THIS LINE. THANK YOU */

#include <stdio.h>
#include <setjmp.h>
#include "atoms.h"

/* macro for bitwise operations */
#ifndef BIT
#define BIT(x) (1ul<<(x))
#endif

/* macro for some compilation time checks */
#ifndef ASSERT
#define ASSERT(e) extern char (*ASSERT(void)) [sizeof(char[1 - 2*!(e)])]
#endif

/* macros for parametric inclusion of header files */
#define MORE_WORDS_FILE_STR(file) #file
#define MORE_WORDS_FILE_INCLUDE(file) MORE_WORDS_FILE_STR(file)

#define CMD_BUFFER_SIZE  256        /* size of the command buffer (including the terminating 0 character) */

#define STACK_DATA  (0xfffff000)    /* data element id modifier indicating stack data */
#define SOURCE      (STACK_DATA-1)  /* compilation source element id */

/* data element id type identifier (lowest 2 bits in an element id of stack type) */
#define TYPE_MASK       3           /* general mask for the type */
#define TYPE_REAL       0           /* floating point number or very big integer */
#define TYPE_UINT       1           /* unsigned integer */
#define TYPE_SINT       2           /* signed integer */
#define TYPE_TEXT       3           /* text or binary data */

/* execution parameters for (newpf) in fanf_execute() */
#define NEWPF_CURRENT   0
#define NEWPF_NEW       1

/* error codes */
/* code 0 cannot be used and ERROR_NONE must be code 1 */
#define ERROR_NONE          1
#define ERROR_BAD_SYNTAX    2
#define ERROR_UNKNOWN_WORD  3
#define ERROR_INTERNAL_ERROR 4
#define ERROR_REDEF_ATOM    5
#define ERROR_REDEF_WORD    6
#define ERROR_CODE_TOO_LONG 7
#define ERROR_NAME_TOO_LONG 8
#define ERROR_UNEXPECTED_END 9
#define ERROR_NAMESPACE_OUT 10
#define ERROR_UNKNOWN_CODE  11
#define ERROR_NO_MEMORY     12
#define ERROR_SHELL_WORD    13
#define ERROR_STACK_EMPTY   14
#define ERROR_MAX_PROCESSES 15
#define ERROR_MEMORY_REF    16
#define ERROR_NESTING       17
#define ERROR_TYPE_MISMATCH 18
#define ERROR_NO_REFERENCE  19
#define ERROR_DIV_ZERO      20
#define ERROR_INVALID_MATH  21
#define ERROR_UNDEFINED_VAR 22
#define ERROR_INVALID_INDEX 23
#define ERROR_ATOM_LISTING  24
#define ERROR_NESTED_CTRL   25
#define ERROR_UNEXP_BREAK   26
#define ERROR_UNEXP_CONT    27
#define ERROR_UNEXP_ELSE    28
#define ERROR_UNEXP_ENDIF   29
#define ERROR_UNEXP_REPEAT  30
#define ERROR_INVALID_LOC   31
#define ERROR_INVALID_FMT   32
#define ERROR_MANY_FILES    33
#define ERROR_UNKNOWN_FILE  34
#define ERROR_STORAGE       35
#define ERROR_BAD_PARAMETER 36

extern char ns[NAMESPACE_SIZE];     /* current namespace */

#define FLAG_COMMENT    BIT(0)  /* a multi-line commentary is currently being written */
#define FLAG_COLON      BIT(1)  /* a : character has been compiled last */
#define FLAG_REFER      BIT(2)  /* a @ character has been compiled last */
#define FLAG_WAS_EOW    BIT(3)  /* a ';' word has been compiled last */
#define FLAG_COMPILE    BIT(4)  /* compilation is currently in process (compilation vs execution errors) */
#define FLAG_KEEP_PCODE BIT(5)  /* do not restore pCode after execution of a command */
#define FLAG_FORCE_CMPL BIT(6)  /* forceful compilation; no direct code is executed */
#define FLAG_RESTR_OK   BIT(7)  /* indicates that the initialrestoring of the FANF library has been performed */
#define FLAG_DIRECT     BIT(8)  /* currently executing in direct mode */

extern unsigned short flags;    /* FANF and FVM flags (see FLAG_xxx constants */

#define PRELOADED_CODES 13      /* total number of the pre-loaded codes */

#define CODE_NOP        0
#define CODE_EOW        1
#define CODE_NEW        2
#define CODE_NFMT       3
#define CODE_IF         4
#define CODE_ELSE       5
#define CODE_ENDIF      6
#define CODE_DO         7
#define CODE_WHILE      8
#define CODE_LOOP       9
#define CODE_REPEAT     10
#define CODE_CONTINUE   11
#define CODE_BREAK      12

extern unsigned char code[PRELOADED_CODES]; /* some important pre-loaded atomic codes */
                                            /* for indexes of the pre-loaded codes see CODE_xxx constants */

/* branch and execution control structure */
enum type_t {BLANK, IFEND, DOREP, WHREP, LPREP};
typedef struct {
    enum type_t type;       /* type of the record: (blank), (if/else/endif), (do/repeat), (while/repeat) or (loop/repeat) */
    long condition;         /* execution condition for forward referencing types: (if/else/endif) and (while/repeat) */
                            /* counter for (do/repeat) */
                            /* variable id for (while/repeat) */
    unsigned int index;     /* variable index for (while/repeat) */
    char contf;             /* 'continue' flag */
    unsigned char *retaddr; /* return address for backwards referencing types: (do/repeat) and (loop/repeat) */
} f_ctrl_t;

extern jmp_buf shell;           /* shell environment */
extern int lines_before_pause;  /* printed lines before pause */
extern char ns[NAMESPACE_SIZE]; /* current namespace */
extern char *fsrc;              /* input source pointer for parsing and compilation; modified along with the compilation process */
extern unsigned char process;   /* current process */
extern unsigned char nested;    /* global nested calls counter */
extern unsigned int stid;       /* pre-calculated stack id for the current process */
extern unsigned char *PC[MAX_PROCESS];  /* process code pointers */
extern unsigned int newdc[MAX_PROCESS]; /* new/; depth counter for each process */
extern unsigned int refid[MAX_PROCESS]; /* refer id for each process during execution */
extern unsigned int refix[MAX_PROCESS]; /* refer indexed for each process during execution */
extern unsigned int refns[MAX_PROCESS]; /* refer namespace for each process during execution (used to create record id for the variables) */
extern unsigned char ctldc[MAX_PROCESS]; /* depth counter for the execution control stack for each process */
extern f_ctrl_t ctrl[MAX_PROCESS][MAX_CTRL];    /* execution control structures for each process */
extern void *files[MAX_FILES];  /* currently open files */
extern unsigned char *taskPC;   /* new task PC */
extern unsigned char *runPC;    /* address of a word which needs to be executed - used by f_run() */
extern unsigned long addPC;     /* this value is added to the PC after the execution of VLN-encoded instructions */

/*
Variable Length Number (VLN) is a method of expressing numbers in variable length according to their magnitude. The trade-off
is that the value 0 can only be expressed in the longest format.
VLN works this way (considering unsigned parameter):
If the number is between 1 and 255, it is expressed simply by its one-byte value;
If the number is greater than 255 but smaller than 65536, it is expressed by 0, followed by the two-byte value (H then L)
If the number is 0 or greater than 65535, it is expressed by 0, 0, followed by the four-byte value (HH, HL, LH, LL)
*/

/*
main FANF shell entry function
input
(*platform) optional string with the platform name
(memsize) desired FANF memory size in number of kilobytes; if 0 the shell will try to allocate the maximum available
*/
void FANF(const char *platform, unsigned long memsize);

/*
print boring stuff (no need to bother learning about this function)
*/
void f_author(const char *platform);

/*
print a new line and a message and wait the user to press [Enter] key if the predefined number of lines has been reached
*/
void new_line(void);

/*
initialise the FVM
*/
void fvm_init(void);

/*
create a binary string from a number
input
(num) the input number
output
bin() the output binary string
*/
char *bin(unsigned long num);

/*
get a VLN-encoded number
input
(*address) address in Fmem[] where the VLN-encoded number starts
(*value) pointer to a buffer where the read number will be saved
output
*get_vln() address pointing after the VLN-encoded number
(*value) read VLN value
*/
unsigned char *get_vln(unsigned char *address, unsigned long *value);

/*
store a number in VLN notation
input
(n) the numeric constant to be evaluated
(*pCode) position in the compiled source for the new pseudocode
output
(*pCode) new value of the pointer in the output code
*/
void pCode_vln(const unsigned long n);

/*
find a specified atom by its name or code
input
(*word) the word as text (terminated by NULL, Tab, CR, LF or space)
        if no word is supplied, the search is performed by code supplied in the second parameter (the function will handle errors itself in this case)
(*code) searched code (only used of (*word) is null or blank)
output
find_atom() pointer to the atom in the description table, or a null pointer if not found
*/
const f_atom_t *find_atom(const char *word, const unsigned char code);

/*
add an atom to the compiled code
input
(*word) the atomic word as text
(*pCode) input compilation pointer
output
(*pCode) updated compilation pointer
*/
void add_atom(const char *word);

/*
find the address of a specified word by its name
input
(*word) the word as text (the ending : character MUST be included)
output
find_word() pointer to the beginning of the word's definition in the FVM code, or a null pointer if not found
(error) error code
*/
const unsigned char *find_word(const char *word);

/*
print a word by its call address
input
(*a) address supplied by %call, %task or %ref instruction
(*cns) optional: current namespace
*/
void print_addr_word(unsigned long a, const char *cns);

/*
compile a single FANF word (word, constant or commentary)
input
(*fsrc) pointer to the input term to be parsed (terminated by NULL, Tab, CR, LF or space)
(*pCode) position in the compiled source for the new pseudocode
output
(*fsrc) new value of the pointer in the input source
(*pCode) new value of the pointer in the output code
*/
void fanf_compile_word(void);

#define MPF_NAMESPACE   0   /* determine the namespace of a specified location */
#define MPF_CLEANUP     1   /* clean up the library from direct words */

/*
multi-purpose function; perforing the oprtation selected by the value in (opr)
input
(*cpos) position whose namespace needs to be determined or (pCode) for cleanup
(opr) select operation to perform (see MPF_xxx constants)
output
(ns[]) actual namespace (only if cleanup operation has been selected)
*/
void fanf_mpf(const unsigned char *cpos, char opr);

/*
execute FANF pseudocode starting from a given address
input
(*pc) entry point for execution
(newpf) 'new process' flag: if raised, a new process is created, otherwise the current process' PC is assigned with the entry address
        see NEWPF_xxx constants
*/
void fanf_execute(const unsigned char *pc, char newpf);

/*
text line editor
input
(*buf) text buffer
(bufsz) maximum size of the text buffer including the terminating 0 character
(x) initial position of the cursor (starting from 0)
output
(*buf) text after editing
*/
void line_edit(char *buf, unsigned int bufsz, int x);

#endif
