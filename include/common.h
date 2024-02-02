#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h> 
#include <time.h> 
#include <utime.h> // For changing file timestamps
#include "ansi_color.h" // for ANSI color codes

typedef unsigned int uint;
typedef unsigned short ushort;
typedef long long llong;
typedef unsigned long long ullong;
typedef unsigned char uchar;
typedef char *String;
typedef char const *constString;

#define ERR_NOERR 0
#define ERR_COMMAND_INVALID 1
#define ERR_ARGS_MISSING 2
#define ERR_FILE_ERROR 3
#define ERR_NOREPO 4
#define ERR_CONFIG_NOTFOUND 5
#define ERR_ALREADY_EXIST 6
#define ERR_MALLOC 7
#define ERR_NOT_EXIST 8
#define ERR_GENERAL 10
#define ERR_DEATACHED_HEAD 11
#define ERR_NOT_COMMITED_CHANGE_FOUND 12
#define ERR_UNKNOWN 100

#define PATH_MAX 4096
#define STR_LINE_MAX 256
#define STR_MAX 1000
#define STR_CMD_MAX 10000
#define COMMIT_MSG_LEN_MAX 72
#define DATETIME_STR_MAX 50

#define DEFAULT_DATETIME_FORMAT "%c %z"

#define checkArgument(num, cmd) (argc >= (num) + 1 && isMatch(argv[num], cmd))
#define checkArgumentPure(num, cmd) (argc == num + 1 && isMatch(argv[num], cmd))
#define checkAnyArgument(cmd)                \
	({                                       \
		int __i;                             \
		for (__i = argc; __i > 0; __i--)     \
			if (isMatch(argv[__i - 1], cmd)) \
				break;                       \
		__i ? __i - 1 : 0;                   \
	})

#define systemf(...) ({char _cmd[STR_CMD_MAX]; sprintf(_cmd, __VA_ARGS__); system(_cmd); })

#define printWarning(...)                  \
	({                                     \
		fprintf(stderr, _YEL __VA_ARGS__); \
		fprintf(stderr, _RST "\n");        \
	})

#define printError(...)                    \
	({                                     \
		fprintf(stderr, _RED __VA_ARGS__); \
		fprintf(stderr, _RST "\n");        \
	})

#define perrExit(errcode, ...)   \
	({                           \
		printError(__VA_ARGS__); \
		exit(errcode);           \
	})

// Useful Macros for Debug :)
#define _DEBUG_(...)                                                                 \
	printf(_BLU_BKG "\tDEBUG (Line %d, File %s):" _DBCOLOR " ", __LINE__, __FILE__); \
	printf(_YEL __VA_ARGS__);                                                        \
	printf(_RST "\n")

#define ADD_EMPTY(array, type)                              \
	if (array.len == 0)                                     \
		array.arr = malloc((array.len = 1) * sizeof(type)); \
	else                                                    \
		array.arr = realloc(array.arr, sizeof(type) * (++array.len));

#define _RST_ERR ({extern int _err; _err = 0; })
#define _ERR ({extern int _err; _err; }) // int _err declared in main.c
#define __retTry ({int __err = _ERR; _RST_ERR; return __err;})

#define tryWith(type, p, val, fail, exep, fre) for (type p = (val); (p && !_RST_ERR) && (p || ((fail), 0)) && p != (void *)1; ({fre; if(_ERR)(exep); p = (void *)1; }))
#define with(p, val, fre) tryWith(typeof(val), p, val, {}, {}, fre)
#define tryWithString(s, val, fail, exep) tryWith(String, s, val, fail, exep, free(s))
#define withString(p, val) tryWithString(p, val, {}, {})
#define tryWithFile(f, path, fail, exep) tryWith(FILE *, f, fopen(path, "rb+"), fail, exep, fclose(f))
#define throw(code) ({extern int _err; _err = code; continue; })

#endif