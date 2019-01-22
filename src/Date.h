#ifndef __Date_h__
#define __Date_h__

#include <ctime>

char* GMTDateTime()
{
    time_t rawTime;
    tm* ptm;
    char* result;

    time(&rawTime);
    ptm = gmtime(&rawTime);

    result = asctime(ptm);
    result[strlen(result)-1] = '\0';
    return result;
}

#endif  /* __Date_h__ */