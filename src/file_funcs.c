/*******************************
 *        file_funcs.c         *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "file_funcs.h"

/////////////////// Functions related to the file contents ////////////////

bool isFilesSame(constString path1, constString path2)
{
	tryWithFile(f1, path1, ({ return false; }), __retTry)
		tryWithFile(f2, path2, throw(false), throw(_ERR))
	{
		size_t size;
		if ((size = GET_FILE_SIZE(f1)) != GET_FILE_SIZE(f2))
			throw(false); // different file sizes

		for (size_t i = 0; i < size; i++)
			if (fgetc(f1) != fgetc(f2))
				throw(false); // found difference

		throw(true);
	}
	return false;
}

Diff getDiff(constString baseFilePath, constString changedFilePath, int f1begin, int f1end, int f2begin, int f2end)
{
	Diff diff = {NULL, NULL, 0, NULL, NULL, 0};

	// Attempt to open the base file
	tryWithFile(baseFile, baseFilePath, ({ return diff; }), ({ return diff; }))
	{
		// Attempt to open the changed file
		tryWithFile(changedFile, changedFilePath, throw(0), throw(0))
		{
			char line1[STR_LINE_MAX], line2[STR_LINE_MAX];
			uint line1_cnt = f1begin - 1, line2_cnt = f2begin - 1;
			SEEK_TO_LINE(baseFile, f1begin);
			SEEK_TO_LINE(changedFile, f2begin);
			char *file1ReadResult = NULL;

			// Compare lines between the specified ranges
			while ((file1ReadResult = SCAN_LINE_BOUNDED(baseFile, line1, line1_cnt, f1end)) && SCAN_LINE_BOUNDED(changedFile, line2, line2_cnt, f2end))
			{
				// If lines are the same, continue to the next iteration
				if (strcmp(line1, line2) == 0) // if same, continue
					continue;

				// If lines are different, record them in the diff structure
				else
				{
					ADD_EMPTY(diff.linesRemoved, diff.removedCount, String);
					diff.removedCount--;
					ADD_EMPTY(diff.lineNumberRemoved, diff.removedCount, uint);
					diff.linesRemoved[diff.removedCount - 1] = strDup(line1);
					diff.lineNumberRemoved[diff.removedCount - 1] = line1_cnt;
					///////////////
					ADD_EMPTY(diff.linesAdded, diff.addedCount, String);
					diff.addedCount--;
					ADD_EMPTY(diff.lineNumberAdded, diff.addedCount, uint);
					diff.linesAdded[diff.addedCount - 1] = strDup(line2);
					diff.lineNumberAdded[diff.addedCount - 1] = line2_cnt;
				}
			}

			// Record any remaining lines in the base file
			while (file1ReadResult || (file1ReadResult = SCAN_LINE_BOUNDED(baseFile, line1, line1_cnt, f1end)))
			{
				ADD_EMPTY(diff.linesRemoved, diff.removedCount, String);
				diff.removedCount--;
				ADD_EMPTY(diff.lineNumberRemoved, diff.removedCount, uint);
				diff.linesRemoved[diff.removedCount - 1] = strDup(line1);
				diff.lineNumberRemoved[diff.removedCount - 1] = line1_cnt;
				file1ReadResult = NULL;
			}

			// Record any remaining lines in the changed file
			while (SCAN_LINE_BOUNDED(changedFile, line2, line2_cnt, f2end))
			{
				ADD_EMPTY(diff.linesAdded, diff.addedCount, String);
				diff.addedCount--;
				ADD_EMPTY(diff.lineNumberAdded, diff.addedCount, uint);
				diff.linesAdded[diff.addedCount - 1] = strDup(line2);
				diff.lineNumberAdded[diff.addedCount - 1] = line2_cnt;
			}
		}
	}

	return diff;
}

void freeDiffStruct(Diff *diff)
{
	// Free memory for arrays storing added lines and their line numbers
	if (diff->addedCount)
	{
		if (diff->lineNumberAdded)
			free(diff->lineNumberAdded);
		if (diff->linesAdded)
		{
			for (size_t i = 0; i < diff->addedCount; ++i)
				free(diff->linesAdded[i]);
			free(diff->linesAdded);
		}
	}

	// Free memory for arrays storing removed lines and their line numbers
	if (diff->removedCount)
	{
		if (diff->lineNumberRemoved)
			free(diff->lineNumberRemoved);
		if (diff->linesRemoved)
		{
			for (size_t i = 0; i < diff->removedCount; ++i)
				free(diff->linesRemoved[i]);
			free(diff->linesRemoved);
		}
	}
}

int fileMemMove(FILE *file, long source, long destination, size_t size)
{
	tryWithString(buffer, (char *)malloc(size), { return ERR_MALLOC; }, {})
	{
		// goto source
		fseek(file, source, SEEK_SET);
		// read from source
		fread(buffer, 1, size, file);
		// goto dest
		fseek(file, destination, SEEK_SET);
		// write in dest
		fwrite(buffer, 1, size, file);
	}

	if (ferror(file))
		return ERR_FILE_ERROR;
	else
		return ERR_NOERR;
}

int searchLine(FILE *file, constString pattern)
{
	int lineNum = 1;
	char buf[STR_LINE_MAX];
	rewind(file);
	for (lineNum = 1; fgets(buf, STR_LINE_MAX - 1, file) != 0; lineNum++)
		if (isMatch(buf, pattern))
		{
			rewind(file);
			return lineNum;
		}

	return -1;
}

int replaceLine(FILE *file, int lineNumber, constString newContent)
{
	if (SEEK_TO_LINE(file, lineNumber) == GET_FILE_SIZE(file)) // Append
	{
		fputs(newContent, file);
		return (ferror(file)) ? ERR_FILE_ERROR : ERR_NOERR;
	}

	long firstPos = ftell(file);
	char buf[STR_LINE_MAX];
	if (fgets(buf, STR_LINE_MAX - 1, file) == NULL)
		return ERR_FILE_ERROR;

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file) - firstPos - strlen(buf);

	// Move data within the file's memory
	fileMemMove(file, firstPos + strlen(buf), firstPos + strlen(newContent), size);

	fseek(file, firstPos, SEEK_SET);
	fputs(newContent, file);

	// Truncate the file to the adjusted size
	ftruncate(fileno(file), size + firstPos + strlen(newContent));

	if (ferror(file))
		return ERR_FILE_ERROR;
	else
		return ERR_NOERR;
}

int removeLine(FILE *file, int lineNumber)
{
	if (SEEK_TO_LINE(file, lineNumber) == GET_FILE_SIZE(file))
		return ERR_FILE_ERROR; // Line does not exist, nothing to remove

	long firstPos = ftell(file);
	char buf[STR_LINE_MAX];
	if (fgets(buf, STR_LINE_MAX - 1, file) == NULL)
		return ERR_FILE_ERROR;

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file) - firstPos - strlen(buf);

	// Move data within the file's memory to remove the line
	fileMemMove(file, firstPos + strlen(buf), firstPos, size);

	// Truncate the file to the adjusted size
	ftruncate(fileno(file), size + firstPos);

	if (ferror(file))
		return ERR_FILE_ERROR;
	else
		return ERR_NOERR;
}

int insertLine(FILE *file, int lineNumber, constString newContent)
{
	SEEK_TO_LINE(file, lineNumber);
	long firstPos = ftell(file);

	// Calculate the size of the remaining content after the insertion point
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file) - firstPos;

	// Move data within the file's memory to make room for the new line
	fileMemMove(file, firstPos, firstPos + strlen(newContent), size);

	// Move back to the insertion point
	fseek(file, firstPos, SEEK_SET);

	// Insert the new line
	fputs(newContent, file);

	if (ferror(file))
		return ERR_FILE_ERROR;
	else
		return ERR_NOERR;
}

int copyFile(constString _src, constString _dest, constString repo)
{
	// Construct full source and destination paths.
	char src[PATH_MAX], dest[PATH_MAX];
	if (repo)
	{
		strcat_s(src, repo, "/", _src);
		strcat_s(dest, repo, "/", _dest);
	}
	else
	{
		strcpy(src, _src);
		strcpy(dest, _dest);
	}

	// Ensure the destination directory exists.
	char dir[PATH_MAX];
	systemf("mkdir -p \"%s\"", getParentName(dir, dest));

	// Remove existing destination file, create an empty file, and open both source and destination files.
	systemf("rm \"%s\" >/dev/null 2>/dev/null", dest);
	systemf("touch \"%s\"", dest);
	tryWithFile(srcFile, src, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		tryWithFile(destFile, dest, throw(ERR_FILE_ERROR), throw(_ERR))
		{
			// Copy file contents in 512-byte chunks.
			size_t size = GET_FILE_SIZE(srcFile);
			for (long i = 0; i < size / 512; i++)
			{
				char sectorBuf[512];
				fread(sectorBuf, 1, 512, srcFile);
				fwrite(sectorBuf, 1, 512, destFile);
			}
			// Copy remaining bytes.
			for (long i = 0; i < size % 512; i++)
			{
				char buf;
				fread(&buf, 1, 1, srcFile);
				fwrite(&buf, 1, 1, destFile);
			}
			// Check for errors during file copying.
			throw(ferror(srcFile) || ferror(destFile));
		}
	}
	return ERR_NOERR;
}

/////////////////// Functions related to the file entries and paths ////////////////

String normalizePath(constString _path, constString _repoPath)
{
	String absolutePath = NULL;
	char buf[PATH_MAX], repoPath[PATH_MAX];

	// Attempt to get the real path of the input path.
	if (realpath(_path, buf) == 0)
	{
		// If realpath fails, assume _path is relative to the CWD or absolute.
		if (isMatch(_path, "/*"))
			strcpy(buf, _path);
		else
		{
			getcwd(buf, PATH_MAX);
			strcat(buf, "/");
			strcat(buf, _path);
		}

		// remove extra trailing '/'
		while ((strlen(buf) > 1) && buf[strlen(buf) - 1] == '/')
			buf[strlen(buf) - 1] = '\0';
	}
	absolutePath = buf;

	// Adjust the absolute path based on the repository path if provided.
	if (_repoPath != NULL && strlen(_repoPath) > 1)
	{
		strcpy(repoPath, _repoPath);

		// remove any trailing '/' from repoPath
		while (repoPath[strlen(repoPath) - 1] == '/')
			repoPath[strlen(repoPath) - 1] = '\0';
		strcat(repoPath, "/"); // add one trailing '/' to end of repoPath

		// remove any trailing '/' from path
		while (buf[strlen(buf) - 1] == '/')
			buf[strlen(buf) - 1] = '\0';
		strcat(buf, "/"); // add one trailing '/' to end of path

		// Check if the path is same with the repository path
		if (!strcmp(buf, repoPath))
			return strDup(".");
		
		// Check if the path is under the repository or not
		if (strstr(buf, repoPath) == NULL)
			return NULL;
		
		// Make the path relative to the repository.
		absolutePath += strlen(repoPath);

		// remove any trailing '/' from result
		while (absolutePath[strlen(absolutePath) - 1] == '/')
			absolutePath[strlen(absolutePath) - 1] = '\0';

		// Ensure a non-empty relative path.
		if (strlen(absolutePath) == 0)
			strcat(absolutePath, ".");
	}

	// Return the dynamically allocated normalized path.
	return strDup(absolutePath);
}

FileEntry getFileEntry(constString _path, constString _repopath)
{
	FileEntry entry;
	entry.path = NULL;

	// Normalize the input path and check if it's within the repository.
	String normalizedPath = normalizePath(_path, _repopath);
	if (!normalizedPath) // Out of repo
		return entry;

	entry.path = normalizedPath;

	// Check if the file or directory exists.
	if (access(_path, F_OK) != 0)
	{
		// Not exist
		entry.isDeleted = 1;
		entry.dateModif = 0;
		entry.isDir = 0;
		entry.permission = 0;
	}
	else
	{
		// Retrieve file/directory information using stat.
		struct stat _fs;
		stat(_path, &_fs);

		entry.path = normalizedPath;
		entry.dateModif = _fs.st_mtime;
		entry.isDir = S_ISDIR(_fs.st_mode);
		entry.isDeleted = 0;
		entry.permission = _fs.st_mode & 0x1FF;
	}

	return entry;
}

void freeFileEntry(FileEntry *array, uint len)
{
	if (array == NULL || len == 0)
		return;
	for (int i = 0; i < len; i++)
		if (array[i].path)
			free(array[i].path);
}

// Comparator function for qsort FileEntries (Directory first, Name Ascending)
int __file_entry_sort_comparator(const void *a, const void *b)
{
	if (((FileEntry *)a)->isDir && !((FileEntry *)b)->isDir)
		return -1000;
	if (!((FileEntry *)a)->isDir && ((FileEntry *)b)->isDir)
		return +1000;
	else
		return strcasecmp(getFileName(((FileEntry *)a)->path), getFileName(((FileEntry *)b)->path));
}

int ls(FileEntry **buf, constString _path)
{
	// Normalize the path and check if it's a valid path
	tryWithString(path, normalizePath(_path, NULL), ({ return -1; }), __retTry)
	{
		// Check if Path not exist
		if (access(path, F_OK) != 0)
			throw(-1);

		// Get file status
		struct stat st;
		stat(path, &st);

		// Check if path is a directory
		if (S_ISDIR(st.st_mode))
		{
			// Open the directory
			tryWith(DIR *, dir, opendir(path), throw(-1), throw(_ERR), closedir(dir))
			{
				// Loop through directory entries
				struct dirent *entry;
				FileEntry *childs = NULL;
				uint countOfChilds = 0;
				while ((entry = readdir(dir)) != NULL)
				{
					// Don't add . and .. to childs
					if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
						continue;

					// Allocate memory for new child
					ADD_EMPTY(childs, countOfChilds, FileEntry);

					// Get absolute path of the child entry
					withString(entryPath, strcat_d(path, "/", entry->d_name))
						// Get and store the file entry details
						childs[countOfChilds - 1] = getFileEntry(entryPath, NULL);
				}

				// Sort the entries
				qsort(childs, countOfChilds, sizeof(FileEntry), __file_entry_sort_comparator);

				// Assign buffer if provided
				if (buf)
					*buf = childs;
				else if (countOfChilds)
					free(childs);

				// Return the count of child entries
				throw(countOfChilds);
			}
		}

		// If path is not a directory, return -2
		else
			throw(-2);
	}
	return 0;
}
