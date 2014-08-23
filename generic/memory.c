#include <string.h>
#include "memory.h"
#include "fanf.h"   /* error codes */

unsigned long FMEM_SIZE=0;  /* detected free memory size in bytes */
unsigned char *Fmem;        /* memory array */
unsigned char *pCode=0;     /* compilation pointer - points to where the next pseudocode will be written; gets changed by the pseudocode generating functions */
unsigned char *pData=0;     /* data pointer - points to the beginning of the data records chain */


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
f_data_t *get_data(unsigned int id, int osc) {
    if(osc<0) osc=0;
    unsigned char *p=pData;
    while((p-Fmem)<FMEM_SIZE) { /* check whether this element already exists in the chain */
        if(((((f_data_t *)p)->id)&(~TYPE_MASK))==(id&(~TYPE_MASK))) {
            if(!osc) break; else osc--;
        }
        p+=(HDR+((f_data_t *)p)->len);
    }
    if((p-Fmem)<FMEM_SIZE) return (f_data_t *)p; else return 0;
}


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
void set_data(unsigned int id, char type, int osc, void *data, long len) {
    id=(id&(~TYPE_MASK))+(type&TYPE_MASK);
    long r;
    unsigned char *p=0;
    if(osc>=0) p=(unsigned char *)get_data(id,osc);

    /* update or delete an existing data element */
    if(p) {
        if(len>=0) {    /* update */
            unsigned char *d=(unsigned char *)data;
            r=(((f_data_t *)p)->len)-len;
            if((pData+r)<=pCode) longjmp(shell,ERROR_NO_MEMORY);
            if(r) {
                memmove(pData+r,pData,p-pData);
                if(d>=pData && d<p) d+=r;
            }
            p+=r;
            ((f_data_t *)p)->id=id;
            ((f_data_t *)p)->len=len;
            memmove(p+HDR,d,len);
            pData+=r;
        } else {        /* delete */
            r=HDR+((f_data_t *)p)->len;
            if(p>pData) memmove(pData+r,pData,p-pData);
            pData+=r;
        }
    }

    /* new data element */
    else {
        if(osc>0) longjmp(shell,ERROR_INVALID_INDEX);  /* trying to set data with invalid index */
        if(len>=0) {
            r=HDR+len;
            if((pData-r)<=pCode) longjmp(shell,ERROR_NO_MEMORY);
            pData-=r;
            ((f_data_t *)pData)->id=id;
            ((f_data_t *)pData)->len=len;
            memmove(pData+HDR,data,len);
        }
    }

}
