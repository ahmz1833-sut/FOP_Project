#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "common.h"
#include "string_funcs.h"

typedef struct
{
	String path;
	time_t dateModif;
	uint permission : 9;
	unsigned isDir : 1;
	unsigned isDeleted : 1;
} FileEntry;

typedef struct
{
	String *linesRemoved;
	uint *lineNumberRemoved;
	uint removedCount;
	//////////////////
	String *linesAdded;
	uint *lineNumberAdded;
	uint addedCount;
} Diff;

#define VALID_CHARS "a-zA-Z0-9._/!@&^,'( ){}-"

#define getFileName(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)
#define getParentName(dest, path) ({char* s=strrchr(path, '/'); int _idx = s?s-path:0; strncpy(dest,path,_idx); dest[_idx]='\0'; dest; })

/**
 * @brief Move the file cursor to a specific line number
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

#define SCAN_LINE_BOUNDED(f, d, cnt, end)                                                                      \
	({                                                                                                         \
		char *c = NULL;                                                                                        \
		while ((c = (cnt < end || end == -1) ? (cnt++, fgets(d, sizeof(d), f)) : NULL) && isEmpty(strtrim(d))) \
			;                                                                                                  \
		c;                                                                                                     \
	})

/**
 * @brief Get the size of a file
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

/**
 * @brief Move data within a file's memory
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

int searchLine(FILE *file, constString pattern);

/**
 * @brief Replace a line in a file with new content
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
 * @brief Remove a line from a file
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
 * @brief Insert a line into a file at the specified position
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
 * @brief Normalize a path relative to a repository path
 *
 * The normalizePath function takes an input path, resolves its absolute path, and then normalizes
 * it relative to a repository path. The resulting normalized path is returned as a newly allocated string.
 *
 * @param _path The input path to be normalized
 * @param _repoPath The repository path for normalization reference
 *@
 * Examples:
 * - Input: normalizePath("/home/user/project/./../project/dir/file.txt", "/home/user/project")
 *   Output: "dir/file.txt"
 * - Input: normalizePath("..", "/user/repo")
 *   Output: NULL
 *
 * @return The normalized path as a newly allocated string. The caller is responsible for freeing
 *         the allocated memory. If an error occurs or the path is invalid or out of repo, NULL is returned.
 */
String normalizePath(constString _path, constString _repoPath);

FileEntry getFileEntry(constString _path, constString _repopath);

/**
 * @brief List entries in a directory and retrieve information about them
 *
 * The ls function lists entries in a specified directory and retrieves information about each entry.
 * It returns an array of FileEntry structures containing details such as name, parent directory, file size,
 * modification date, whether it's a directory or a symbolic link, and permissions. The function also
 * returns the total count of entries in the specified directory.
 *
 * @param buf A pointer to an array of FileEntry structures to store information about directory entries
 * @param _path The path of the directory to list entries
 *
 * Example:
 * - Input: ls(&entries, "/home/user/project")
 *   Output: Retrieves information about entries in the "/home/user/project" directory and stores
 *           the details in the 'entries' array. Returns the total count of entries.
 *
 * @return The total count of entries retrieved and stored in the 'buf' array. Returns -1 if an error
 *         occurs, such as the specified directory not existing or being inaccessible.
 * 			if the input path is a file, returns -2.
 */
int ls(FileEntry **buf, constString path);

int copyFile(constString _src, constString _dest, constString repo);

Diff getDiff(constString baseFilePath, constString changedFilePath, int f1begin, int f1end, int f2begin, int f2end);
void freeDiffStruct(Diff *diff);

void freeFileEntry(FileEntry *array, uint len);

#endif