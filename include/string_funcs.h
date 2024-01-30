#ifndef __STR_PROC_H__
#define __STR_PROC_H__

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "common.h"

/**
 * @brief Check if a string contains '*' or '?'
 *
 * The hasWildcard macro checks if a given string contains the wildcard characters '*'
 * or '?'. It returns true if either character is found, otherwise false.
 *
 * @param str The input string to check for wildcards
 *
 * Example:
 * - Input: hasWildcard("file*.txt")
 *   Output: true
 *
 * @return true if the string contains '*' or '?', false otherwise.
 */
#define hasWildcard(str) (strchr(str, '*') || strchr(str, '?'))

/**
 * @brief Concatenate constant strings using a macro and return a newly allocated string
 *
 * The strConcat macro uses the better_strcat_impl function to concatenate constant strings
 * and returns a newly allocated string. It accepts a variable number of arguments.
 *
 * Example:
 * - Input: strConcat("Hello, ", "World", "!")
 *   Output: "Hello, World!"
 *
 * @param ... Constant strings to be concatenated
 *
 * @return A newly allocated string containing the concatenated result. The caller is
 *         responsible for freeing the allocated memory.
 */
#define strConcat(...) better_strcat_impl(__VA_ARGS__, (String)NULL)

#define strConcatStatic(dest, first, ...) _static_strcat_impl(dest, first, __VA_ARGS__, (String)NULL)

String _static_strcat_impl(String dest, constString first, ...);

/**
 * @brief Check if a string is empty after removing leading and trailing whitespaces
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
 * @brief Validate a string based on allowed characters
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
 * @brief Trim leading and trailing whitespaces in a string
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
 * @brief Tokenize a string using a delimiter and store the tokens in an array
 *
 * The tokenizeString function takes a string and a delimiter as input parameters,
 * splits the given string into tokens using the provided delimiter, and returns the
 * number of tokens extracted. The resulting tokens are stored in the provided destination array.
 *
 * @param str The input string to be tokenized
 * @param delim The delimiter used for tokenization
 * @param destArray The array to store the resulting tokens
 *
 * Example:
 * - Input: tokenizeString("apple,orange,banana", ",", tokens)
 *   Output: destArray[0] = "apple", destArray[1] = "orange", destArray[2] = "banana"
 *
 * @return The total number of tokens found in the string.
 */
uint tokenizeString(String str, constString delim, String *destArray);

/**
 * @brief Duplicate a string in memory
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
 * @brief Concatenate constant strings and return a newly allocated string
 *
 * The better_strcat_impl function concatenates constant strings and returns a newly
 * allocated string. The function accepts a variable number of arguments, where the last
 * argument must be NULL, indicating the end of the string list.
 *
 * @param first The first constant string to be concatenated
 * @param ... Additional constant strings, terminated with NULL
 *
 * Example:
 * - Input: better_strcat_impl("Hello, ", "World", "!", NULL)
 *   Output: "Hello, World!"
 *
 * @return A newly allocated string containing the concatenated result. The caller is
 *         responsible for freeing the allocated memory.
 */
String better_strcat_impl(constString first, ...);

/**
 * @brief Check if a given text matches a (probably) wildcard-included pattern
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

#endif