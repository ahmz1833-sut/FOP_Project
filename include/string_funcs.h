/*******************************
 *      string_funcs.h         *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
********************************/
#ifndef __STR_FUNCS_H__
#define __STR_FUNCS_H__

#include "common.h"
#include <string.h>
#include <ctype.h>
#include <langinfo.h> // used in strptime implementation
#include <stddef.h> // used in strptime implementation

/*!
 * @brief Concatenate constant strings using a macro and return a newly allocated string (string_funcs.h)
 *
 * The strcat_d macro uses the _dynamic_strcat_impl function to concatenate constant strings
 * and returns a newly allocated string. It accepts a variable number of arguments.
 *
 * Example:
 * - Input: strcat_d("Hello, ", "World", "!")
 *   Output: "Hello, World!"
 *
 * @param ... Constant strings to be concatenated
 *
 * @return A newly allocated string containing the concatenated result. The caller is
 *         responsible for freeing the allocated memory.
 */
#define strcat_d(...) _dynamic_strcat_impl(__VA_ARGS__, (String)NULL)

/*!
 * @brief Concatenate multiple strings into a destination string (static version). (string_funcs.h)
 *
 * The strcat_s macro calls the _static_strcat_impl function to concatenate multiple strings
 * into the destination string in a static manner.
 *
 * Usage:
 * @code{.c}
 * char result[256]; // Ensure enough space for the concatenated content
 * strcat_s(result, "Hello", " ", "world");
 * @endcode
 *
 * @param dest The destination string to store the result.
 * @param ... Strings to concatenate.
 * @return The destination string with concatenated content.
 *
 * @note The destination string must have enough space to accommodate the concatenated content.
 * @note The macro provides a convenient way to concatenate strings in a static manner.
 */
#define strcat_s(dest, ...) _static_strcat_impl(dest, __VA_ARGS__, (String)NULL)


/**
 * @brief Make a string bold. (string_funcs.h)
 *
 * The boldText function adds bold formatting to the provided string.
 *
 * @param s The input string to be formatted in bold.
 * @return A new string with bold formatting applied.
 *
 * @note The returned string is dynamically allocated and should be freed by the caller.
 */
String boldText(constString s);


/**
 * @brief Trim leading and trailing whitespaces in a string  (string_funcs.h)
 *
 * The strtrim function removes leading and trailing whitespaces, including
 * newline, carriage return, tab, form feed, and space characters, from the given string.
 *
 * @param s The input string to be trimmed
 *
 * Example:
 * - Input: "   Hello, World!   "
 *   Output: "Hello, World!"
 *
 * @return The trimmed string. The original string is modified in-place.
 */
String strtrim(String s);


/**
 * @brief Check if a string is empty after removing leading and trailing whitespaces (string_funcs.h)
 *
 * The isEmpty function takes a string as input, trims leading and trailing whitespaces,
 * and then checks if the resulting string is empty. The function returns true if the string
 * is empty, and false otherwise.
 *
 * @param s The input string to check for emptiness
 *
 * @return true if the trimmed string is empty or the string is NULL, false otherwise.
 */
bool isEmpty(constString s);


/**
 * @brief Duplicate a string in memory (string_funcs.h)
 *
 * The strDup function takes a constant string as input, allocates memory for a duplicate,
 * copies the content of the input string into the newly allocated memory, and returns the
 * pointer to the duplicated string.
 *
 * @param src The constant string to be duplicated
 *
 * @return The pointer to the duplicated string. The caller is responsible for freeing
 *         the allocated memory.
 */
String strDup(constString src);


/**
 * @brief Replace exact words in a text based on a word pattern and apply a replacement function if provided.
 * (string_funcs.h)
 *
 * The strReplace function tokenizes the input text, identifies exact words matching the specified word pattern,
 * and replaces them using a custom replacement function if one is provided. The modified text is stored in the
 * destination buffer.
 *
 * @param dest Destination buffer to store the changed text. Must be pre-allocated by the caller. 
 *				 (if NULL, just find target and return number of matchings)
 * @param text Input text in which words will be replaced.
 * @param wordPattern Pattern to match exact words for replacement.
 * @param replaceFunction Custom function to replace matched words. Can be null if no replacement is needed.
 *
 * @return The number of words matched with the word pattern.
 *
 * @note
 * - The caller is responsible for allocating enough space for the destination string (dest).
 * - If the dest NULL proveided, this function only returns the count of matches without applying any changes.
 * - The replaceFunction must allocate a new string for the replacement.
 * - If the replaceFunction pointer is null, the original text remains unchanged.
 *
 * @note 
 * - Example:
 * Given text = "Hello world! Replace hello with hi.", wordPattern = "hello", and replaceFunction = strToUpper,
 * the resulting dest will be "HI world! Replace HI with hi.".
 */
int strReplace(String dest, constString text, constString wordPattern, String (*replaceFunction)(constString));


/**
 * @brief Validate a string based on allowed characters  (string_funcs.h)
 *
 * The strValidate function takes a string and a set of allowed characters and
 * retains the valid characters in the string while removing the invalid ones.
 *
 * @param dest The output string containing valid characters. If NULL, only the
 *             count of invalid characters will be returned.
 * @param str The input string to be validated
 * @param allowedChars The string of allowed characters, including allowed ranges
 *
 * Ranges are defined as [a-z] for allowing characters between a and z, and ^[a-z]
 * for disallowing characters between a and z. Example:
 * - "[a-zA-Z0-9]" allows all uppercase and lowercase letters, and digits.
 * - "^[#*\@^&/\\%<>]" disallows the characters '#', '*', '\@', '^', '&', '/', '\\', '%', and '<'.
 *
 * @return The total count of invalid characters removed from the string.
 *         If dest is NULL, only the count is returned, and no modifications
 *         are made to the input string.
 */
uint strValidate(String dest, constString str, constString allowedChars);


/**
 * @brief Convert an unsigned long long (ullong) number to a hexadecimal string. (string_funcs.h)
 *
 * The toHexString function converts an unsigned long long (ullong) number to a hexadecimal string.
 * The resulting string is left-padded with zeros to achieve the specified number of digits.
  * Example:
 * - Input: toHexString(255, 4)
 *   Output: "00FF"
 *
 * @param number The unsigned long long number to convert.
 * @param digits The desired number of digits in the hexadecimal string.
 *
 *
 * @return The hexadecimal string representation of the number.
 *
 * @note The caller is responsible for freeing the allocated memory of the returned string.
 */
String toHexString(ullong number, int digits);


/**
 * @brief Tokenize a string using a delimiter and store the tokens in an array (string_funcs.h)
 *
 * @note The tokenizeString function takes a string and a delimiter as input parameters,
 * splits the given string into tokens using the provided delimiter, and returns the
 * number of tokens extracted. The resulting tokens are stored in the provided destination array.
 *
 * @param str The input string to be tokenized
 * @param delim The delimiter used for tokenization
 * @param destArray The array to store the resulting tokens
 *
 * @note Example:
 * - Input: tokenizeString("apple,orange,banana", ",", tokens)
 *   Output: destArray[0] = "apple", destArray[1] = "orange", destArray[2] = "banana"
 *
 * @return The total number of tokens found in the string.
 */
uint tokenizeString(String str, constString delim, String *destArray);


/**
 * @brief Check if a given text matches a (probably) wildcard-included pattern (string_funcs.h)
 *
 * The isMatch function checks if a given text matches a pattern that may include
 * wildcard characters ('*' and '?'). It trims leading and trailing whitespaces from
 * both the text and pattern, then performs the matching process.
 *
 * @param _text The input text to be checked
 * @param _pattern The pattern to match against the text
 *
 * Example:
 * - Input: isMatch("hello.txt", "*.txt")
 *   Output: true
 *
 * @return true if the text matches the pattern, false otherwise.
 */
bool isMatch(constString _text, constString _pattern);


/**
 * @brief Parse a string representation of time according to a format string. (implementation opaque)
 *
 * The strptime function parses a string (s) representing time according to the format string (f)
 * and fills the Time structure (tm) with the extracted information.
 *
 * @param s The input string to be parsed.
 * @param f The format string specifying the expected format of the input string.
 * @param tm The Time structure to store the parsed time information.
 * @return A pointer to the first character not processed in the input string, or NULL if an error occurs.
 *
 * @note The function supports a subset of the format specifiers used in the standard C library's strptime function.
 * @note The caller is responsible for allocating the Time structure (tm).
 */
String strptime(constString s, constString f, struct tm *tm);


/**
 * @brief Parse a date and time string in various formats and return the corresponding time_t value. (string_funcs.h)
 *
 * The parseDateTimeAuto function attempts to parse the input date and time string (dateTimeStr) in multiple formats,
 * and returns the corresponding time_t value. The supported formats include "YYYY-MM-DD HH:MM:SS",
 * "YYYY/MM/DD HH:MM:SS", "YYYY-MM-DD", and "YYYY/MM/DD".
 *
 * @param dateTimeStr The input date and time string to be parsed.
 * @return The time_t value representing the parsed date and time, or ERR_ARGS_MISSING if an error occurs.
 *
 * @note The function internally uses the strptime and mktime functions.
 * @note If the input string does not match any of the supported formats, the function returns ERR_ARGS_MISSING.
 *
 */
time_t parseDateTimeAuto(constString dateTimeStr);


/**
 * @brief Concatenate constant strings and return a newly allocated string  (string_funcs.h)
 *
 * The _dynamic_strcat_impl function concatenates constant strings and returns a newly
 * allocated string. The function accepts a variable number of arguments, where the last
 * argument must be NULL, indicating the end of the string list.
 *
 * @param first The first constant string to be concatenated
 * @param ... Additional constant strings, terminated with NULL
 *
 * @note Use the related macro 'strcat_d(...)' for automatic NULL terminating.
 *
 * @return A newly allocated string containing the concatenated result. The caller is
 *         responsible for freeing the allocated memory.
 */
String _dynamic_strcat_impl(constString first, ...);


/**
 * @brief Concatenate multiple strings into a destination string. (string_funcs.h)
 *
 * The _static_strcat_impl function concatenates multiple strings into the destination string.
 *
 * @param dest The destination string to store the result.
 * @param first The first string to concatenate.
 * @param ... Additional strings to concatenate. The last argument must be NULL.
 * @return The destination string with concatenated content.
 *
 * @note The destination string must have enough space to accommodate the concatenated content.
 * @note Use the related macro
 */
String _static_strcat_impl(String dest, constString first, ...);

#endif