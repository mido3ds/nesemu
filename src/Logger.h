#ifndef __Logger_h__
#define __Logger_h__

#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <string>

using std::string;

char* getGMTDateTime() {
    time_t rawTime;
    tm* ptm;
    char* result;

    time(&rawTime);
    ptm = gmtime(&rawTime);

    result = asctime(ptm);
    result[strlen(result)-1] = '\0';
    return result;
}

void logInfo(string format, ...) {
    format = string("[INFO][") + string(getGMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logWarning(string format, ...) {
    format = string("[WARNING][") + string(getGMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logError(string format, ...) {
    format = string("[ERROR][") + string(getGMTDateTime()) + "]: " + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stderr, format.c_str(), args);
    va_end (args);
}

#endif  /* __Logger_h__ */