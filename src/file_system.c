#include "file_system.h"

String normalizePath(constString _path, constString _repoPath)
{
	String absolutePath = NULL;
	char buf[PATH_MAX];
	if (realpath(_path, buf) == 0)
	{
		if(isMatch(_path, "/*"))
			strcpy(buf, _path);
		else
		{
			getcwd(buf, PATH_MAX);
			strcat(buf, _path);
		}
	}
	absolutePath = buf;
	if (_repoPath != NULL && strlen(_repoPath) > 1)
	{
		if (strstr(buf, _repoPath) == NULL) return NULL;
		absolutePath += strlen(_repoPath) + 1;
		if (strlen(absolutePath) == 0)
			strcat(absolutePath, ".");
	}
	return strDup(absolutePath);
}

// input path must be absolute or relative to cwd!!
// The path in returned FileEntry is relative to Repo root
FileEntry getFileEntry(constString _path, constString _repopath)
{
	FileEntry entry;
	entry.path = NULL;

	String normalizedPath = normalizePath(_path, _repopath);
	if (!normalizedPath) // Out of repo
		return entry;
	
	entry.path = normalizedPath;

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

// input path must be absolute or relative to cwd
// list the childs into a dir, put into buf. (buf paths are absolute)
int ls(FileEntry **buf, constString _path)
{
	tryWithString(path, normalizePath(_path, ""), ({ return -1; }), ({ return _ERR; }))
	{
		// if Path not exist
		if (access(path, F_OK) != 0)
			throw(-1);

		struct stat st;
		stat(path, &st);

		// if path is a directory -> obtain childs entries and put into buf
		if (S_ISDIR(st.st_mode))
		{
			tryWith(DIR *, dir, opendir(path), throw(-1), throw(_ERR), closedir(dir))
			{
				struct dirent *entry;
				FileEntry *childs;
				uint countOfChilds = 0;
				while ((entry = readdir(dir)) != NULL)
				{
					// Don't add . and .. to childs
					if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
						continue;

					if (countOfChilds == 0)
						childs = (FileEntry *)malloc((countOfChilds = 1) * sizeof(FileEntry));
					else
						childs = (FileEntry *)realloc(childs, (++countOfChilds) * sizeof(FileEntry));

					withString(entryPath, strConcat(path, "/", entry->d_name))
						childs[countOfChilds - 1] = getFileEntry(entryPath, NULL);
				}
				if (buf)
					*buf = childs;
				throw(countOfChilds);
			}
		}

		// if path is not a directory -> return -2
		else
			throw(-2);
	}
}

void freeFileEntry(FileEntry *array, uint len)
{
	if (array == NULL || len == 0)
		return;
	for (int i = 0; i < len; i++)
		if (array[i].path)
			free(array[i].path);
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
	// return (removeLine(file, lineNumber) || insertLine(file, lineNumber, newContent));

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
	char src[PATH_MAX], dest[PATH_MAX];
	strConcatStatic(src, repo, "/", _src);
	strConcatStatic(dest, repo, "/", _dest);

	char dir[PATH_MAX];
	systemf("mkdir -p \"%s\"", getParentName(dir, dest));
	systemf("touch \"%s\"", dest);

	tryWithFile(srcFile, src, ({ return ERR_FILE_ERROR; }), ({ return _ERR; }))
	{
		tryWithFile(destFile, dest, throw(ERR_FILE_ERROR), throw(_ERR))
		{
			size_t size = GET_FILE_SIZE(srcFile);
			for (long i = 0; i < size / 512; i++)
			{
				char sectorBuf[512];
				fread(sectorBuf, 1, 512, srcFile);
				fwrite(sectorBuf, 1, 512, destFile);
			}
			for (long i = 0; i < size % 512; i++)
			{
				char buf;
				fread(&buf, 1, 1, srcFile);
				fwrite(&buf, 1, 1, destFile);
			}
			throw(ferror(srcFile) || ferror(destFile));
		}
	}
}

typedef struct
{

} Diff;

Diff *getDiff(String path1, String path2)
{
	Diff *diff = malloc(sizeof(Diff));

	return NULL;
}