#pragma once

#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

using std::string;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __FILENAME__ __FILE__ + SOURCE_PATH_SIZE
void setPath(string path, size_t size, int line);

#ifndef DEBUG
void logInfo(string format, ...);
#define INFO(...) setPath(__FILE__, SOURCE_PATH_SIZE, __LINE__); logInfo(__VA_ARGS__)
#else
#define INFO(...)
#endif

#ifndef IGNORE_WARNINGS
void logWarning(string format, ...);
#define WARNING(...) setPath(__FILE__, SOURCE_PATH_SIZE, __LINE__); logWarning(__VA_ARGS__)
#else
#define WARNING(...)
#endif

void logError(string format, ...);
#define ERROR(...) setPath(__FILE__, SOURCE_PATH_SIZE, __LINE__); logError(__VA_ARGS__)
