#pragma once

#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

using std::string;

#ifndef TEST
void logInfo(string format, ...);
#else
#define logInfo(...)
#endif

void logWarning(string format, ...);

void logError(string format, ...);
