#include "file_system.h"

String normalizePath(constString _path, constString _repoPath)
{
	String absolutePath = NULL;
	tryWithString(buf, malloc(PATH_MAX), ({ return NULL; }), ({ return NULL; }))
	{
		absolutePath = buf;
		if (realpath(_path, buf) == 0)
			throw(0);

		if (_repoPath != NULL && strlen(_repoPath) > 1)
		{
			if (strstr(buf, _repoPath) == NULL)
				throw(0);
			absolutePath += strlen(_repoPath) + 1;
			if (strlen(absolutePath) == 0)
				strcat(absolutePath, ".");
		}
		absolutePath = strDup(absolutePath);
	}
	return absolutePath;
}

FileEntry getFileEntry(constString _path, constString _repopath)
{
	FileEntry entry;
	entry.path = NULL;

	String normalizedPath = normalizePath(_path, _repopath);
	if (!normalizedPath)
		goto err;

	struct stat _fs;
	stat(_path, &_fs);

	entry.path = normalizedPath;
	entry.fileSize = _fs.st_size;
	entry.dateModif = _fs.st_mtime;
	entry.isDir = S_ISDIR(_fs.st_mode);
	entry.isLink = S_ISLNK(_fs.st_mode);
	entry.permission = _fs.st_mode & 0x1FF;
err:
	return entry;
}

int ls(FileEntry **buf, constString _path)
{
	tryWithString(path, normalizePath(_path, "/"), ({  return -1; }), ({  return _ERR; }))
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

	// if (!(path = ))
	// 	return -1;

	// 	if (access(path, F_OK))
	// 		goto err;

	// 	struct stat st;
	// 	stat(path, &st);

	// 	// if path is a directory -> obtain childs entries and put into buf
	// 	if (S_ISDIR(st.st_mode))
	// 	{
	// 		DIR *dir = opendir(path);
	// 		if (dir == NULL)
	// 			goto err;
	// 		struct dirent *entry;
	// 		FileEntry *childs;
	// 		uint countOfChilds = 0;
	// 		while ((entry = readdir(dir)) != NULL)
	// 		{
	// 			// Don't add . and .. to childs
	// 			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
	// 				continue;

	// 			if (countOfChilds == 0)
	// 				childs = (FileEntry *)malloc((countOfChilds = 1) * sizeof(FileEntry));
	// 			else
	// 				childs = (FileEntry *)realloc(childs, (++countOfChilds) * sizeof(FileEntry));

	// 			String entryPath = strConcat(path, "/", entry->d_name);
	// 			childs[countOfChilds - 1] = getFileEntry(entryPath, NULL);
	// 			free(entryPath);
	// 		}
	// 		closedir(dir);
	// 		free(path);
	// 		if (buf)
	// 			*buf = childs;
	// 		return countOfChilds;
	// 	}

	// 	// if path is not a directory -> return -2
	// 	else
	// 	{
	// 		free(path);
	// 		return -2;
	// 	}

	// err:
	// 	free(path);
	// 	return -1;
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