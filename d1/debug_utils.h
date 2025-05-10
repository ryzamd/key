#pragma once
#include <stdio.h>

void DebugLog(const char* message) {
    FILE* f = fopen("D:\\keylogger_debug.txt", "a");
    if (f) {
        fprintf(f, "%s\n", message);
        fclose(f);
    }
}