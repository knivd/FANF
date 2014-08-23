/*
 platform-dependend functions
 when porting this is the only code to be rewritten
 */

#ifndef PLATFORM_H
#define	PLATFORM_H

#define PLATFORM "PC"

/*
store the library into file
NOTE: the name of this function must not be changed
*/
void _fanf_store(void);

/*
restore the library from file
NOTE: the name of this function must not be changed
*/
void _fanf_restore(void);

#endif
