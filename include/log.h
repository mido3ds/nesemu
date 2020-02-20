#pragma once

#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

using std::string;

#ifndef TEST
void logInfo(string format, ...);
#define INFO logInfo
#else
#define INFO(...)
#endif

#ifndef IGNORE_WARNINGS
void logWarning(string format, ...);
#define WARNING logWarning
#else
#define WARNING(...)
#endif

void logError(string format, ...);
#define ERROR logError