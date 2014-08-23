#ifndef MEMORY_H
#define MEMORY_H

/* data chain record header structure; the actual data starts immediately after the header */
typedef struct {
    unsigned int id;    /* data element id (see explanation below) */
    unsigned int len;   /* data length in bytes */
} __attribute__ ((packed)) f_data_t;

extern unsigned long FMEM_SIZE; /* detected free memory size */
extern unsigned char *Fmem;     /* memory array */
extern unsigned char *pCode;    /* compilation pointer - points to where the next pseudocode will be written; gets changed by the pseudocode generating functions */
extern unsigned char *pData;    /* data pointer - points to the beginning of the data records chain (the records are of f_data_t type) */

#define HDR (sizeof(f_data_t))

/*
element id for data containers (%refer instruction) is the very address of the word
element id for stack elements has the following format: $fffff000+(process<<2)+(modifier flags)
(process) is a 10-bit value
(data type): 2 bits, see TYPE_xxx constants
*/

/*
get a data element
input
(id) element id
(osc) 'occurence skip counter'; instructs the function how many occurences of the same id (lowest two bits masked) to skip before returning a found element
        values smaller than 0 are considered as 0
(*pData) input state of the data pointer
output
get_data() pointer to the element of 0 if not found
*/
f_data_t *get_data(unsigned int id, int osc);

/*
add/update/delete a data element
input
(id) element id
(type) 2 bits element type; these two bits replace the lowest two bits of the id
(osc) 'occurence skip counter'; instructs the function how many occurences of the same id (lowest two bits masked) to skip before returning a found element
        values smaller than 0 will force a new element to be added unconditionally
(*data) pointer to the actual data to be stored
(len) data length; if this value is smaller than 0, the entire element is deleted
(*pData) input state of the data pointer
output
(*pData) update data pointer
(error) error code
*/
void set_data(unsigned int id, char type, int osc, void *data, long len);

#endif
