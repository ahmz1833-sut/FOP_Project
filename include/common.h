#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

typedef struct tm Time;
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
#define ERR_44 8
#define ERR_UNKNOWN 100

#define STR_LINE_MAX 256
#define STR_MAX 1000

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
#define _DEBUG_(...)                                                                   \
	printf("\n" _YEL_BKG "\tDEBUG (Line %d, File %s): " _DBCOLOR, __LINE__, __FILE__); \
	printf(_YEL __VA_ARGS__)

#define _RST_ERR ({extern int _err; _err = 0;})
#define _ERR ({extern int _err; _err;})

#define tryWith(type, p, val, fail, exep, fre) for (type p = (val); (p && !_RST_ERR) && (p || ((fail), 0)) && p != (void *)1; ({fre; if(_ERR)(exep); p = (void *)1; }))
#define with(p, val, fre) tryWith(typeof(val), p, val, {}, {}, fre)
#define tryWithString(s, val, fail, exep) tryWith(String, s, val, fail, exep, free(s))
#define withString(p, val) tryWithString(p, val, {}, {})
#define tryWithFile(f, path, fail, exep) tryWith(FILE *, f, fopen(path, "rb+"), fail, exep, fclose(f))
#define throw(code) ({extern int _err; _err = code; continue; })

#define is_or(x, y) == (x) || ({ return (y); 0; })
#define is_not_or(x, y) != (x) || ({ return (y); 0; })
#define Is(x) == (x) || ({ printError("failure at %s:%d", __FILE__, __LINE__); exit(EXIT_FAILURE); 0; })
#define Is_not(x) != (x) || ({ printError("failure at %s:%d", __FILE__, __LINE__); exit(EXIT_FAILURE); 0; })

#define must_use __attribute__((warn_unused_result))

#endif