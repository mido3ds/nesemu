#include "log.h"

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define RESET   "\033[0m"

string _path;

void setPath(string path, size_t size, int line) {
    _path = path.substr(size, path.size()-size).c_str() + string(":") + std::to_string(line);
}

void logInfo(string format, ...) {
    format = GREEN"[INFO " + _path + RESET + "] " + format + '\n';

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format.c_str(), args);
    va_end(args);
}

void logWarning(string format, ...) {
    format = YELLOW"[WARNING " + _path + "] " + format + RESET"\n";

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format.c_str(), args);
    va_end(args);
}

void logError(string format, ...) {
    format = RED"[ERROR " + _path + "] "+ format + RESET"\n";

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format.c_str(), args);
    va_end(args);
}
