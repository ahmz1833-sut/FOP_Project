#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "datatypes.h"
#include "ansi_color.h"
#include "file_system.h"
#include "install.h"

// Useful Macros for Debug :)
#define _DEBUG_(fmt, val1, val2, val3) printf("\n\tDEBUG (Line %d): ", __LINE__); printf(fmt, val1, val2, val3)
#define _DEBUG1_(x, fmt)               printf("\n\tDEBUG (Line %d): %s=", __LINE__, #x); printf(fmt, x)
#define _DEBUG2_(x, fmtx, y, fmty)     printf("\n\tDEBUG (Line %d): %s=", __LINE__, #x); printf(fmtx, x); printf(" / %s=", #y); printf(fmty, y);

#endif