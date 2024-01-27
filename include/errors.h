#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <stdarg.h>

#define ERR_NOERR 0
#define ERR_COMMAND_INVALID 1
#define ERR_ARGS_MISSING 2
#define ERR_FILE_ERROR 3
#define ERR_NOREPO 4
#define ERR_CONFIG_NOTFOUND 5
#define ERR_ALREADY_EXIST 6
#define ERR_33 7
#define ERR_44 8
#define ERR_UNKNOWN 100

#define printWarning(...)                          \
    ({                                             \
        fprintf(stderr, _SGR_YELLOWF __VA_ARGS__); \
        fprintf(stderr, _SGR_RESET "\n");          \
    })

#define printError(...)                         \
    ({                                          \
        fprintf(stderr, _SGR_REDF __VA_ARGS__); \
        fprintf(stderr, _SGR_RESET "\n");       \
    })

#define perrExit(errcode, ...)   \
    ({                           \
        printError(__VA_ARGS__); \
        exit(errcode);           \
    })

#endif