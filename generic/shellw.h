#ifndef SHELLW_H
#define SHELLW_H

/*
basic help information
*/
void fanf_help(void);

/*
list currently compiled FANF words
*/
void fanf_words(void);

/*
[w] list
list a given word or if no parameter is supplied - the entire library
*/
void fanf_list(void);

/*
display all stack data
NOTE: does not change anything in the stack
*/
void fanf_peek(void);

/*
[w] insp
inspect a specified data container or if no parameter is supplied - all data containers
*/
void fanf_insp(void);

/*
x s resrc
replace source line starting from index x (given by the 'list' word) with the new source s
*/
void fanf_resrc(void);

/*
x s insrc
insert a new source line s starting from index x (given by the 'list' word)
*/
void fanf_insrc(void);

/*
a l dump
hexadecimal dump of l bytes starting from address a (mainly for debug)
*/
void fanf_dump(void);

#endif
