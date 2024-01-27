#ifndef __INSTALL_H__
#define __INSTALL_H__

#include "header.h"

#define PROGRAM_PATH "/usr/bin/" PROGRAM_NAME
#define INSTALL_NEOGIT(src) __install_neogit_linux(src)
#define REMOVE_NEOGIT() __remove_neogit_linux()
int __install_neogit_linux(const char* source);
int __remove_neogit_linux();

#define CHECK_EXIST(s) (system("ls " s " >/dev/null 2>&1") == 0)
#define IS_INSTALLED() CHECK_EXIST(PROGRAM_PATH)

int promptInstallation(const char* srcAddress);
int promptUninstallation();

#endif