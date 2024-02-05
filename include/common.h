/*******************************
 *         common.h            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>
#include <time.h>
#include <utime.h>		// For changing file timestamps
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
#define ERR_GENERAL 9
#define ERR_DEATACHED_HEAD 10
#define ERR_NOT_COMMITED_CHANGE_FOUND 11
#define ERR_CONFLICT 12
#define ERR_UNKNOWN 100

#define PATH_MAX 4096
#define STR_LINE_MAX 5000
#define STR_MAX 10000
#define STR_CMD_MAX 10000
#define COMMIT_MSG_LEN_MAX 72
#define DATETIME_STR_MAX 50

#define DEFAULT_DATETIME_FORMAT "%c %z"

// Check the argument number (num) to match with (cmd)
#define checkArgument(num, cmd) (argc >= (num) + 1 && isMatch(argv[num], cmd))

// Check the argument number (num) to match with (cmd) and is there no extra arguments
#define checkArgumentPure(num, cmd) (argc == num + 1 && isMatch(argv[num], cmd))

// Check all arguments and find (cmd) and return its index
#define checkAnyArgument(cmd)                \
	({                                       \
		int __i;                             \
		for (__i = argc; __i > 0; __i--)     \
			if (isMatch(argv[__i - 1], cmd)) \
				break;                       \
		__i ? __i - 1 : 0;                   \
	})

/**
 * @brief Execute a shell command using formatted input. (common.h)
 * The systemf macro takes a format string and arguments, similar to printf,
 * and constructs a command string. It then executes the command using the system function.
 * @param ... The format string and arguments for constructing the shell command.
 * Example:
 * - Input: systemf("echo %s", "Hello, World!");
 *   Output: Executes the shell command 'echo Hello, World!'.
 * @return The result of the system function.
 */
#define systemf(...) ({char _cmd[STR_CMD_MAX]; sprintf(_cmd, __VA_ARGS__); system(_cmd); })

// Prints a formatted line + LF in stderr in yellow color
#define printWarning(...)                  \
	({                                     \
		fprintf(stderr, _YEL __VA_ARGS__); \
		fprintf(stderr, _RST "\n");        \
	})

// Prints a formatted line + LF in stderr in red color
#define printError(...)                    \
	({                                     \
		fprintf(stderr, _RED __VA_ARGS__); \
		fprintf(stderr, _RST "\n");        \
	})

// Prints a formatted error red, and exit with code.
#define perrExit(errcode, ...)   \
	({                           \
		printError(__VA_ARGS__); \
		exit(errcode);           \
	})

// Useful Macro for Debug :)
#define _DEBUG_(...)                                                                 \
	printf(_BLU_BKG "\tDEBUG (Line %d, File %s):" _DBCOLOR " ", __LINE__, __FILE__); \
	printf(_YEL __VA_ARGS__);                                                        \
	printf(_RST "\n")

/**
 * @brief Add an empty element to a dynamic array. (common.h)
 *
 * The ADD_EMPTY macro adds an empty element to a dynamic array. If the counter is initially
 * less than or equal to zero, it allocates memory for one element. Otherwise, it reallocates
 * memory to accommodate one more element in the array.
 *
 * @param array The dynamic array to which an empty element is added.
 * @param counter The counter tracking the number of elements in the array.
 * @param type The data type of the elements in the array.
 *
 * @note The caller is responsible for managing the memory of the dynamic array.
 */
#define ADD_EMPTY(array, counter, type)                 \
	if (counter <= 0)                                   \
		array = malloc(((counter) = 1) * sizeof(type)); \
	else                                                \
		array = realloc(array, sizeof(type) * (++(counter)));

/**
 * @brief Reset the error flag to zero.
 * The _RST_ERR macro resets the global error flag to zero.
 */
#define _RST_ERR ({ extern int _err; _err = 0; })

/**
 * @brief Get the current error code.
 * The _ERR macro retrieves the current error code from the global error flag.
 * The global error flag should be declared in the main.c file.
 */
#define _ERR ({ extern int _err; _err; })

/**
 * @brief Set the current error code.
 * The global error flag should be declared in the main.c file.
 */
#define _SET_ERR(x) ({ extern int _err; _err=x; })

/**
 * @brief Return the current error code and reset the error flag.
 * The __retTry macro is used inside functions with error handling.
 * It retrieves the current error code, resets the error flag, and returns the error code.
 */
#define __retTry ({ int __err = _ERR; _RST_ERR; return __err; })

/**
 * @brief Try block with error handling for pointers.
 * The tryWith macro is a block for error handling with pointers.
 * It takes a pointer variable, a value to initialize the pointer, a failure block, an exception block,
 * and a cleanup block for freeing resources associated with the pointer.
 */
#define tryWith(type, p, val, fail, exep, fre) \
	for (type p = (val); (p && !_RST_ERR) && (p || ((fail), 0)) && p != (void *)1; ({ fre; if (_ERR) (exep); p = (void *)1; }))

/**
 * @brief Try block for managing resources associated with a pointer.
 * The with macro is a simplified version of the tryWith macro.
 * It is used for managing resources associated with a pointer.
 */
#define with(p, val, fre) tryWith(typeof(val), p, val, {}, {}, fre)

/**
 * @brief Try block with error handling for strings.
 * The tryWithString macro is a specific case of tryWith for handling strings.
 * It takes a String variable, a value to initialize the string, a failure block, and an exception block.
 * It also includes a cleanup block for freeing the string resource.
 */
#define tryWithString(s, val, fail, exep) tryWith(String, s, val, fail, exep, free(s))

/**
 * @brief Try block for managing resources associated with a string.
 * The withString macro is a simplified version of the tryWithString macro.
 * It is used for managing resources associated with a string.
 */
#define withString(p, val) tryWithString(p, val, {}, {})

/**
 * @brief Try block with error handling for file operations.
 * The tryWithFile macro is a specific case of tryWith for handling file operations.
 * It takes a FILE pointer variable, a file path to open, a failure block, and an exception block.
 * It also includes a cleanup block for closing the file.
 */
#define tryWithFile(f, path, fail, exep) tryWith(FILE *, f, fopen(path, "rb+"), fail, exep, fclose(f))

/**
 * @brief Throw an error and exit the try/with block
 * The throw macro sets the global error flag to the specified error code and exit the try/with block
 * @note Important: Pay attention when using this macro inside nested loops!
 * @note Use only in root of a try block!
 */
#define throw(code) ({ extern int _err; _err = code; continue; })

#endif