#pragma once

#include <ctime>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>

using std::string;

char* getGMTDateTime();

void logInfo(string format, ...);

void logWarning(string format, ...);

void logError(string format, ...);