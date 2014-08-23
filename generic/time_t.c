#include <string.h>
#include "time_t.h"

#define YEAR_SECONDS 31536000   /* number of seconds in a normal year */
#define DAY_SECONDS 86400       /* number of seconds in a single day */

const char month_length[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
const char month_name[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};


char isLeap(unsigned short year) {
    return((year%400==0 || (year%100!=0 && year%4==0))? 1 : 0);
}


void packTime(date_time_t *dt) {
    if(dt->hour>23 || dt->minute>59 || dt->second>59 || dt->day>31 || dt->month<1 || dt->month>12 || dt->year<1970 || dt->year>2105) {
        dt->utc=0xffffffff;
        return; /* error */
    }
    char mlen[12];
    memcpy(mlen,month_length,sizeof(month_length));
    if (isLeap(dt->year) && mlen[1]<month_length[1]) mlen[1]++; /* correct the length of February in leap years */
    if (dt->day>mlen[dt->month-1]) {
        dt->utc=0xffffffff;
        return; /* error */
    }
    dt->utc=0;
    unsigned short t;
    for (t=1970; t<dt->year; t++) { /* calculate the number of seconds until 1/1/[year] 0:00:00 */
        dt->utc+=YEAR_SECONDS;
        if (isLeap(t)) dt->utc+=DAY_SECONDS;    /* make the correction for leap years */
    }
    for (t=1; t<dt->month; t++) dt->utc+=(mlen[t-1]*DAY_SECONDS);   /* add the number of seconds since the begin of the current year until 1/[month]/[year], 0:00:00 */
    dt->utc+=((dt->day-1)*DAY_SECONDS); /* add the number of seconds passed since the begin of the current month until [day]/[month]/[year], 0:00:00 */
    dt->utc+=((dt->hour*3600)+(dt->minute*60)+dt->second);  /* finally add the seconds passed since the begin of the day */
}


void unpackTime(date_time_t *dt) {
    unsigned long tm=dt->utc;
    dt->year=2105;
    dt->month=12;
    dt->day=31;
    dt->hour=23;
    dt->minute=59;
    dt->second=59;
    dt->millisecond=0;
    packTime(dt);
    if (tm>dt->utc) {
        dt->year=dt->month=dt->day=dt->hour=dt->minute=dt->second=0;
        return;    /* error in the input parameter */
    }
    dt->month=1;
    dt->day=1;
    dt->hour=0;
    dt->minute=0;
    dt->second=0;
    packTime(dt);
    while (dt->utc>tm) {
        dt->year--;
        packTime(dt);
    }
    dt->month=12;
    packTime(dt);
    while (dt->utc>tm) {
        dt->month--;
        packTime(dt);
    }
    dt->day=31;
    packTime(dt);
    while (dt->utc>tm) {
        dt->day--;
        packTime(dt);
    }
    dt->hour=23;
    packTime(dt);
    while (dt->utc>tm) {
        dt->hour--;
        packTime(dt);
    }
    dt->minute=59;
    packTime(dt);
    while (dt->utc>tm) {
        dt->minute--;
        packTime(dt);
    }
    dt->second=59;
    packTime(dt);
    while (dt->utc>tm) {
        dt->second--;
        packTime(dt);
    }
}


void addTime(date_time_t *dt, long sa) {
    char mlen[12];
    memcpy(mlen,month_length,sizeof(month_length));
    if (isLeap(dt->year) && mlen[1]<month_length[1]) mlen[1]++; /* correct the length of February in leap years */
    long y,m,d,h,n,s;
    y=(long)dt->year;
    m=(long)dt->month;
    d=(long)dt->day;
    h=(long)dt->hour;
    n=(long)dt->minute;
    s=(long)dt->second+sa;
    while (s>=60) {
        s-=60;
        n++;
    }
    while (s<0) {
        s+=60;
        n--;
    }
    while (n>=60) {
        n-=60;
        h++;
    }
    while (n<0) {
        n+=60;
        h--;
    }
    while (h>=24) {
        h-=24;
        d++;
    }
    while (h<0) {
        h+=24;
        d--;
    }
    while (d>mlen[m-1]) {
        d-=mlen[m-1];
        m++;
    }
    while (d<1) {
        m--;
        if (m<1) m=12;
        d+=mlen[m-1];
    }
    while (m>12) {
        m-=12;
        y++;
    }
    while (m<1) {
        m+=12;
        y--;
    }
    dt->year=(unsigned short)y;
    dt->month=(unsigned char)m;
    dt->day=(unsigned char)d;
    dt->hour=(unsigned char)h;
    dt->minute=(unsigned char)n;
    dt->second=(unsigned char)s;
}


void clockTime(date_time_t *dt, unsigned short freq) {
    if (dt->valid!=VALID_TIME || freq==0 || freq>1000) return;
    dt->millisecond+=(1000/freq);
    while (dt->millisecond>=1000) {
        dt->millisecond-=1000;
        dt->utc++;
        if (++dt->second>59) {
            dt->second=0;
            if (++dt->minute>59) {
                dt->minute=0;
                if (++dt->hour>23) {
                    char ml=month_length[dt->month-1];
                    if (dt->month==2 && isLeap(dt->year)) ml++; /* correct the length of February in leap years */
                    dt->hour=0;
                    if (++dt->day>ml) {
                        dt->day=1;
                        if (++dt->month>12) {
                            dt->month=1;
                            if (++dt->year>99) dt->year=0;
                        }
                    }
                }
            }
        }
    }
}
