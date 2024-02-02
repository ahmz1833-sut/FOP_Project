/*******************************
 *        file_funcs.c         *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
********************************/
#ifndef __FILE_FUNCS_H__
#define __FILE_FUNCS_H__

#include "common.h"
#include <unistd.h> 
#include <dirent.h>
#include <sys/stat.h>
#include "string_funcs.h"

// A Useful strcuture for representing files (real files or virtual objects in repo) (file_funcs.h)
typedef struct _file_entry_t
{
	String path;
	time_t dateModif;
	uint permission : 9;
	unsigned isDir : 1;
	unsigned isDeleted : 1;
} FileEntry;

// Diff structure (file_funcs.h)
typedef struct _diff_t
{
	String *linesRemoved;
	uint *lineNumberRemoved;
	uint removedCount;
	//////////////////
	String *linesAdded;
	uint *lineNumberAdded;
	uint addedCount;
} Diff;

// Valid characters used in filename validations , etc (file_funcs.h)
#define VALID_CHARS "a-zA-Z0-9._/!@&^,'( ){}-"

/////////////////// Macro Functions ////////////////

/**
 * @brief Extract the filename from a given file path. (file_funcs.h)
 * The getFileName macro extracts the filename from a given file path.
 * Example:
 * - Input: getFileName("/path/to/file.txt")
 *   Output: "file.txt"
 * 
 * @param path The file path from which to extract the filename.
 *
 * @return A pointer to the extracted filename within the provided path.
 */
#define getFileName(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)

/**
 * @brief Extract the parent directory name from a given file path. (file_funcs.h)
 *
 * The getParentName macro extracts the parent directory name from a given file path.
 * 
 * Example:
 * - Input: getParentName(buffer, "/path/to/file.txt")
 *   Output: "path/to"
 *
 * @param dest The pre-allocated destination string to store the parent directory name.
 * @param path The file path from which to extract the parent directory name.
 *
 * @return A pointer to the extracted parent directory name within the provided path.
 *
 * @note The caller is responsible for pre-allocating enough space in the destination string (dest).
 */
#define getParentName(dest, path) ({char* s=strrchr(path, '/'); int _idx = s?s-path:0; strncpy(dest,path,_idx); dest[_idx]='\0'; dest; })

/**
 * @brief Move the file cursor to a specific line number (file_funcs.h)
 *
 * The SEEK_TO_LINE macro sets the file cursor to the beginning of the file and navigates
 * to the specified line number in a text file, counting lines from the start. The function
 * returns the total number of characters that were skipped to reach the desired line.
 *
 * @param f The FILE pointer to the target file
 * @param n The line number to seek to (one-based)
 *
 * @return The total number of characters skipped to reach the specified line.
 *         The line number is one-based, meaning the first line is 1, the second line is 2, and so on.
 */
#define SEEK_TO_LINE(f, n)                               \
	({                                                   \
		rewind(f);                                       \
		int _k = 0;                                      \
		for (int _i = 0, _c = 0; _i < n - 1; _i++, _k++) \
			while ((_c = fgetc(f)) != EOF && _c != '\n') \
				_k++;                                    \
		_k;                                              \
	})

/**
 * @brief Scan lines from a file pointer with a specified buffer size and boundary. (file_funcs.h)
 *
 * The SCAN_LINE_BOUNDED macro reads lines from a file pointer until a non-empty line is encountered.
 * It allows scanning within a specified line counter or until the end of the file (end = -1).
 *
 * Example:
 * - Input: SCAN_LINE_BOUNDED(file, buffer, lineCnt, 10)
 *   Output: A non-empty line from the file, and lineCnt is incremented.
 * 
 * @param f        The file pointer to read from.
 * @param d        The buffer to store the read line.
 * @param cnt      The line counter (updated during scanning).
 * @param end      The boundary for scanning lines. If set to -1, scanning continues until the end of the file.
 *
 * @return A pointer to the read line or NULL if the end of the file (or end boundary) is reached or an error occurs.
 */
#define SCAN_LINE_BOUNDED(f, d, cnt, end)                                                                      \
	({                                                                                                         \
		char *c = NULL;                                                                                        \
		while ((c = (cnt < end || end == -1) ? (cnt++, fgets(d, sizeof(d), f)) : NULL) && isEmpty(strtrim(d))) \
			;                                                                                                  \
		c;                                                                                                     \
	})

/**
 * @brief Get the size of a file (file_funcs.h)
 *
 * The GET_FILE_SIZE macro calculates the size of the specified file by moving the
 * file cursor to the end of the file and then back to its original position.
 *
 * @param f The FILE pointer to the target file
 *
 * @return The size of the file in bytes.
 */
#define GET_FILE_SIZE(f)           \
	({                             \
		long _push = ftell(f);     \
		fseek(f, 0, SEEK_END);     \
		long _size = ftell(f);     \
		fseek(f, _push, SEEK_SET); \
		_size;                     \
	})

/////////////////// Functions related to the file contents ////////////////

/**
 * @brief Check if the content of two files is the same. (file_funcs.h)
 *
 * The isFilesSame function compares the content of two files, specified by their paths.
 * It returns true if the files have the same content and false otherwise.
 *
 * @param path1 The path to the first file <<absolute or relative to the current working directory>>
 * @param path2 The path to the second file <<absolute or relative to the current working directory>>
 *
 * @return true if the files have the same content, false otherwise.
 */
bool isFilesSame(constString path1, constString path2);

/**
 * @brief Get the difference between two files within specified line ranges. (file_funcs.h)
 *
 * The getDiff function compares two files, starting from the specified line numbers,
 * within the given line ranges. It identifies and records the lines that are added or removed.
 * The result is stored in a Diff structure.
 *
 * @param baseFilePath    The path to the base file.
 * @param changedFilePath The path to the changed file.
 * @param f1begin         The starting line number in the base file.
 * @param f1end           The ending line number in the base file (use -1 for all lines).
 * @param f2begin         The starting line number in the changed file.
 * @param f2end           The ending line number in the changed file (use -1 for all lines).
 *
 * @return A Diff structure containing the added and removed lines along with their line numbers.
 */
Diff getDiff(constString baseFilePath, constString changedFilePath, int f1begin, int f1end, int f2begin, int f2end);

/**
 * @brief Frees the memory allocated for a Diff structure.
 *
 * The freeDiffStruct function deallocates the memory used by a Diff structure,
 * including arrays storing added and removed lines along with their line numbers.
 *
 * @param diff A pointer to the Diff structure to be freed.
 */
void freeDiffStruct(Diff *diff);

/**
 * @brief Move data within a file's memory (file_funcs.h)
 *
 * The fileMemMove function moves a block of data within a file's memory. It reads
 * data from the source position, seeks to the destination position, and writes the
 * data to the new location. The function returns an error code indicating the success
 * or failure of the operation.
 *
 * @param file The FILE pointer to the target file
 * @param source The source position in the file from which to read the data
 * @param destination The destination position in the file to which the data will be written
 * @param size The size of the data block to be moved, in bytes
 *
 * @return An error code indicating the success or failure of the file memory move operation.
 *         ERR_NOERR is returned for success, while ERR_FILE_ERROR indicates an error.
 */
int fileMemMove(FILE *file, long source, long destination, size_t size);

/**
 * @brief Search for a line containing a specific pattern in a file. (file_funcs.h)
 *
 * The searchLine function searches for a line containing the specified pattern
 * within the given file. If a line with the pattern is found, it returns the line number;
 * otherwise, it returns -1.
 *
 * @param file    The file to search within.
 * @param pattern The pattern to search for in the lines of the file.
 *
 * @return The line number containing the pattern, or -1 if not found.
 */
int searchLine(FILE *file, constString pattern);

/**
 * @brief Replace a line in a file with new content (file_funcs.h)
 *
 * The replaceLine function replaces the content of a specified line in the file with
 * new content. If the specified line number exceeds the existing line count, it appends
 * the new content at the end of the file. The function returns an error code indicating
 * the success or failure of the operation.
 *
 * @param file The FILE pointer to the target file
 * @param lineNumber The line number to be replaced (one-based)
 * @param newContent The new content to replace the existing line
 *
 * @return An error code indicating the success or failure of the line replacement operation.
 *         ERR_NOERR is returned for success, while ERR_FILE_ERROR indicates an error.
 */
int replaceLine(FILE *file, int lineNumber, constString newContent);

/**
 * @brief Remove a line from a file (file_funcs.h)
 *
 * The removeLine function removes the specified line from the file. It shifts the
 * subsequent lines upward to fill the gap. The function returns an error code indicating
 * the success or failure of the operation.
 *
 * @param file The FILE pointer to the target file
 * @param lineNumber The line number to be removed (one-based)
 *
 * @return An error code indicating the success or failure of the line removal operation.
 *         ERR_NOERR is returned for success, while ERR_FILE_ERROR indicates an error.
 */
int removeLine(FILE *file, int lineNumber);

/**
 * @brief Insert a line into a file at the specified position (file_funcs.h)
 *
 * The insertLine function inserts a new line into the file at the specified position.
 * It shifts the subsequent lines downward to make room for the new line. The function
 * returns an error code indicating the success or failure of the operation.
 *
 * @param file The FILE pointer to the target file
 * @param lineNumber The line number at which to insert the new line (one-based)
 * @param newContent The content of the new line to be inserted
 *
 * @return An error code indicating the success or failure of the line insertion operation.
 *         ERR_NOERR is returned for success, while ERR_FILE_ERROR indicates an error.
 */
int insertLine(FILE *file, int lineNumber, constString newContent);

/**
 * @brief Copy a file from the source path to the destination path. (file_funcs.h)
 *
 * The copyFile function copies the contents of a source file to a destination file. If the
 * destination file already exists, it is overwritten. The paths are constructed based on the
 * provided repository path.
 * Example:
 * - Input: copyFile("file.txt", "backup/file_copy.txt", "/path/to/repo")
 *   This copies the contents of "/path/to/repo/file.txt" to "/path/to/repo/backup/file_copy.txt".
 
 * @param _src The relative path of the source file. <<must be relative to provided repopath>>
 * @param _dest The relative path of the destination file. <<must be relative to provided repopath>>
 * @param repo The base path of the repository. <<must be absolute>>
 *
 * @return ERR_NOERR on success, or an error code if an issue occurs during the copy operation.

 * @note - If the source file does not exist, ERR_FILE_ERROR is returned.
 * @note - If there is an issue during file copying, ERR_FILE_ERROR is returned.
 * @note - The destination file is overwritten if it already exists.
 * @note - The caller is responsible for providing valid paths and ensuring that the repository exists.
 */
int copyFile(constString _src, constString _dest, constString repo);

/////////////////// Functions related to the file entries and paths ////////////////

/**
 * @brief Normalize a path relative to a repository path (file_funcs.h)
 *
 * The normalizePath function takes a file path, either relative to the CWD or absolute, and
 * normalizes it to an absolute path. The result is a dynamically allocated string containing the
 * normalized path. The normalization considers the provided repository path to make the path
 * relative to the repository if possible.
 *
 * @param _path The input path to be normalized
 * @param _repoPath The absolute path of the repository. if NULL -> absolute path of _path will be returned
 *
 * Examples:
 * - Input: normalizePath("/home/user/project/./../project/dir/file.txt", "/home/user/project")
 *   Output: "dir/file.txt"
 * - Input: normalizePath("..", "/user/repo")
 *   Output: NULL
 * 
 * @note - If the input path is already absolute, it remains unchanged.
 * @note - If the input path is relative, it is normalized relative to the provided repository path.
 * @note - If the repository path is provided and the input path is not under the repository, NULL is returned.
 * @note - The caller is responsible for freeing the allocated memory for the result.
 * 
 * @return The normalized path as a newly allocated string. The caller is responsible for freeing
 *         the allocated memory. If an error occurs or the path is invalid or out of repo, NULL is returned.
 */
String normalizePath(constString _path, constString _repoPath);

/**
 * @brief Get information about a file or directory entry. (file_funcs.h)
 * The getFileEntry function retrieves information about a file or directory specified by the input path.
 * The returned FileEntry structure contains details such as the relative path to the repository root,
 * modification date, directory status, deletion status, and file permissions.
 * 
 * @param _path The path of the file or directory. It must be either <<absolute or relative to the current working directory>>
 * @param _repopath The absolute path of the repository. if be NULL >> output path will be absolute
 *
 * @return A FileEntry structure containing information about the specified file or directory.
 *
 * @note - If the file or directory does not exist, isDeleted is set to true, and other fields are set to default values.
 * @note - The path in the returned FileEntry is relative to the repository root. (absolute path if _repopath NULL provided)
 * @note - The caller is responsible for freeing the allocated memory for the FileEntry path.
 */
FileEntry getFileEntry(constString _path, constString _repopath);

/**
 * @brief Free memory allocated for an array of FileEntry structures. (file_funcs.h)
 * The freeFileEntry function deallocates memory allocated for the path field in each FileEntry structure
 * within the specified array. It ensures proper cleanup to prevent memory leaks.
 *
 * @param array The array of FileEntry structures.
 * @param len The length of the array.
 * 
 * @note - This function is designed to work with arrays of FileEntry structures, where each structure has an allocated path.
 * @note - It does not free the memory occupied by the array itself, only the allocated paths within each structure.
 */
void freeFileEntry(FileEntry *array, uint len);

/**
 * @brief List the entries in a directory and return an array of FileEntry structures. (file_funcs.h)
 * The ls function lists the entries (files and subdirectories) in the specified directory and returns
 * an array of FileEntry structures representing these entries. The array is sorted by entry names.
 *
 * @param buf Pointer to the array of FileEntry structures to be populated. (Can be NULL if only the count is needed.)
 * @param _path The absolute or relative path to the directory.
 * @return The number of entries in the directory on success, or a negative value on failure.
 *
 * @note - The paths inside buf entries (output of function) << are always absolute.>>
 * @note - The function returns -1 if the specified path does not exist or if there is an issue accessing it.
 * @note - The function returns -2 if the specified path is not a directory.
 * @note - The caller is responsible for freeing the memory allocated for the array and its FileEntry structures.
 * @note - If buf is NULL, only the count of entries will be returned, and no memory will be allocated.
 */
int ls(FileEntry **buf, constString path);

#endif