#ifndef __OS_TIME_H
#define __OS_TIME_H

#include <time.h>

#define DEFAULT_BASE_YEAR   1900

#ifdef __cplusplus
extern "C"
{
#endif

long long get_real_time_usec(void);
long long get_real_time_msec(void);
int  get_real_time_sec(void);
int  get_real_time_sec_of_day(void);
int  get_real_time_weekday(void);
void get_common_time(char *buffer);
void sleep_usec(unsigned int value);
void sleep_msec(unsigned int value);

#ifdef __cplusplus
}
#endif

#endif /* __OS_TIME_H */

