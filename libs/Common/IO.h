#ifndef IO_H_
#define IO_H_

#include <cstdio>

#include "Common/Structs.h"

long long SizeOfFile(const char* filename);
char* ReadToBuf(const char* filename, FILE* file, size_t filesize);

void DoBufRead(FILE* file, const char* filename, FileInfo* Info);

#endif  // IO_H_
