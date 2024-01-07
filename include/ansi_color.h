#ifndef __ANSI_COLOR_H__
#define __ANSI_COLOR_H__

#include <stdio.h>
#ifdef _WIN32
    #include <windows.h>
#endif

#define _SGR_RESET      "\e[m"

#define _SGR_BOLD       "\e[1m"
#define _SGR_NORM       "\e[22m"
#define _SGR_DIM        "\e[2m"

#define _SGR_ITALIC     "\e[3m"
#define _SGR_NOITALIC   "\e[23m"

#define _SGR_UNDERL     "\e[4m"
#define _SGR_NOUNDERL   "\e[24m"

#define _SGR_SHOWBLK    "\e[5m"
#define _SGR_RAPIDBLK   "\e[6m"
#define _SGR_NOBLK      "\e[25m"

#define _SGR_INVERT     "\e[7m"
#define _SGR_NOINVERT   "\e[27m"

#define _SGR_STRIKE     "\e[9m"
#define _SGR_NOSTRIKE   "\e[29m"

#define _SGR_BLACKF     "\e[30m"
#define _SGR_BLACKB     "\e[40m"
#define _SGR_REDF       "\e[31m"
#define _SGR_REDB       "\e[41m"
#define _SGR_GREENF     "\e[32m"
#define _SGR_GREENB     "\e[42m"
#define _SGR_YELLOWF    "\e[33m"
#define _SGR_YELLOWB    "\e[43m"
#define _SGR_BLUEF      "\e[34m"
#define _SGR_BLUEB      "\e[44m"
#define _SGR_MAGENF     "\e[35m"
#define _SGR_MAGENB     "\e[45m"
#define _SGR_CYANF      "\e[36m"
#define _SGR_CYANB      "\e[46m"
#define _SGR_WHITEF     "\e[37m"
#define _SGR_WHITEB     "\e[47m"
#define _SGR_DEF_FG     "\e[39m"
#define _SGR_DEF_BG     "\e[49m"

#define _SAVE_CURSOR    "\e[s"
#define _REST_CURSOR    "\e[u"

void prepareANSI(void);

#endif