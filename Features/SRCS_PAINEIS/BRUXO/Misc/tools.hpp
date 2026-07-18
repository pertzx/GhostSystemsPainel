#pragma once

#include <sys/un.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
typedef unsigned long ulong;

inline uintptr_t get_module_base(int pid, const char *module_name)
{
FILE *fp;
uintptr_t addr = 0;
char *pch;
char filename[32];
char line[1024];
snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
fp = fopen(filename, "r");
if (fp != NULL)
{
while (fgets(line, sizeof(line), fp))
{
if (strstr(line, module_name))
{
pch = strtok(line, "-");
addr = (uintptr_t)strtoul(pch, NULL, 16);
break;
}
}
fclose(fp);
}
return addr;
}

long findLibrary(const char *library) {
char filename[0xff] = {0},
buffer[1024] = {0};
FILE *fp = nullptr;
long address = 0;
sprintf(filename, "/proc/self/maps");
fp = fopen(filename, "rt");
if (fp == nullptr) {
perror("fopen");
goto done;
}
while (fgets(buffer, sizeof(buffer), fp)) {
if (strstr(buffer, library)) {
address = (long) strtoul(buffer, nullptr, 16);
goto done;
}
}
done:
if (fp) {
fclose(fp);
}
return address;
}

uintptr_t getLibraryLoaded(const char *libraryName) {
char line[512] = {0};
FILE *file = fopen(OBFUSCATE("/proc/self/maps"), OBFUSCATE("rt"));
if (file != nullptr) {
while (fgets(line, sizeof(line), file)) {
std::string a = line;
if (strstr(line, libraryName)) {
return true;
}
}
fclose(file);
}
return false;
}
long ClibBase;
long getAbsoluteAddress(const char *libraryName, long relativeAddr) {
if (ClibBase == 0) {
ClibBase = findLibrary(libraryName);
if (ClibBase == 0) {
ClibBase = 0;
}
}
return ClibBase + relativeAddr;
}
