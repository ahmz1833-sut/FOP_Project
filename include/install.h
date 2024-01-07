#ifndef __INSTALL_H__
#define __INSTALL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // For sleep
#include "datatypes.h"
#include "ansi_color.h"
#include "file_system.h"

#define PROGRAM_NAME "neogit"

#ifdef _WIN32 // For Windows

#include <windows.h>
#define PROGRAM_PATH "C:\\Windows\\" PROGRAM_NAME ".exe"
#define INSTALL_NEOGIT(src) __install_neogit_win32(src)
#define REMOVE_NEOGIT() __remove_neogit_win32()
int __install_neogit_win32(const char* source);
int __remove_neogit_win32();

#else // For Linux

#define PROGRAM_PATH "/usr/bin/" PROGRAM_NAME
#define INSTALL_NEOGIT(src) __install_neogit_linux(src)
#define REMOVE_NEOGIT() __remove_neogit_linux()
int __install_neogit_linux(const char* source);
int __remove_neogit_linux();

#endif

#define IS_INSTALLED() CHECK_EXIST(PROGRAM_PATH)

int promptInstallation(const char* srcAddress);
int promptUninstallation();

#endif