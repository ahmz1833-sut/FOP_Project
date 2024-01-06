#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include "datatypes.h"

#ifdef _WIN32 // For Windows

    #include <windows.h>
    #include <Shellapi.h>
    #define PROGRAM_PATH "C:\\Windows\\neogit.exe"
    #define CHECK_EXIST(s) (system("dir " s " >null 2>&1") == 0)

#else // For Linux

    #define PROGRAM_PATH "/usr/bin/neogit"
    #define CHECK_EXIST(s) (system("ls " s " >/dev/null 2>&1") == 0)
    
#endif

int runAsAdmin(String command);

#endif