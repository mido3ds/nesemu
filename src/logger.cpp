#include "logger.h"

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define RESET   "\033[0m"

static char* getGMTDateTime() {
    time_t rawTime;
    tm* ptm;
    char* result;

    time(&rawTime);
    ptm = gmtime(&rawTime);

    result = asctime(ptm);
    result[strlen(result)-1] = '\0';
    return result;
}

#ifndef TEST
void logInfo(string format, ...) {
    format = GREEN"[INFO] "RESET + format + '\n';

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}
#endif

void logWarning(string format, ...) {
    format = YELLOW"[WARNING] " + format + RESET"\n";

    va_list args;
    va_start (args, format);
    vfprintf (stdout, format.c_str(), args);
    va_end (args);
}

void logError(string format, ...) {
    format = RED"[ERROR] " + format + RESET"\n";

    va_list args;
    va_start (args, format);
    vfprintf (stderr, format.c_str(), args);
    va_end (args);
}
