#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__



#ifdef _WIN32 // For Windows

#define CHECK_EXIST(s) (system("dir " s " >nul 2>&1") == 0)

#else // For Linux

#define CHECK_EXIST(s) (system("ls " s " >/dev/null 2>&1") == 0)

#endif

#endif