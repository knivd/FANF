/******************************************************************/
/* time conversions and functions                                 */
/* $ 2006 Konstantin Dimitrov                                     */
/******************************************************************/

#ifndef TIME_T_H
#define TIME_T_H

#define VALID_TIME 0xaa55

/* length of months in a normal (non-leap) year */
extern const char month_length[12];

/* standard 3-character month names */
extern const char month_name[12][4];

/* date/time structure */
typedef struct {
    unsigned short valid;   /* (VALID_TIME) if the information in the record is valid */
    unsigned short millisecond;
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    unsigned long utc;      /* universal time in C */
} __attribute__ ((packed)) date_time_t;

/*
return 1 if the parameter is a leap year and 0 otherwise
input
(year) year to check
output
function()  0 - normal year
            1 - leap year
*/            
char isLeap(unsigned short year);

/*
pack time in UTC value
input
(dt) date/time structure
        .year - 1970..2105
        .month - 1..12
        .day - day 1..(28..31) depending on the month
        .hour - hour 0..23
        .minute - minute 0..59
        .second - second 0..59
output
(dt).utc - packed value (0xffffffff if there was an error)
*/
void packTime(date_time_t *dt);

/*
unpack time from UTC value to its components
input
(dt).utc - packed date/time
output
(dt) unpacked time (will return all zeros if there was an error)
*/
void unpackTime(date_time_t *dt);

/*
add (or subtract) time from extracted time variables
input
(dt) date/time structure
        .year - 1970..2105
        .month - 1..12
        .day - day 1..(28..31) depending on the month
        .hour - hour 0..23
        .minute - minute 0..59
        .second - second 0..59
(sa) number seconds to add/subtract from the time
output
year, month, day, hour, minute, second - corrected values
*/
void addTime(date_time_t *dt, long sa);

/*
full time clock
NOTE: this function must be regularly called from code outside
NOTE: time is not counted if the *dt structure has no .valid flag set with (VALID_TIME) value
NOTE: for best accuracy the calling frequency must be selected so (1000/freq) must not produce a remainder
input
(dt) date/time structure
(freq) calling frequency (number of calls to this function per second; must not exceed 1000)
*/
void clockTime(date_time_t *dt, unsigned short freq);

#endif
