#include "file_system.h"

String normalizePath(constString _path, constString _repoPath)
{
	String buf = malloc(PATH_MAX);
	String absolutePath = buf;
	if (realpath(_path, buf) == 0)
		goto err;
	if (strlen(_repoPath) > 1)
	{
		if (strstr(buf, _repoPath) == NULL)
			goto err;
		absolutePath += strlen(_repoPath) + 1;
		if (strlen(absolutePath) == 0)
			strcat(absolutePath, ".");
	}
	absolutePath = strDup(absolutePath);
	free(buf);
	return absolutePath;
err:
	free(buf);
	return NULL;
}

int ls(Entry **buf, constString _path)
{
	String path;
	if (!(path = normalizePath(_path, "/")))
		return -1;

	// if Path not exist
	if (access(path, F_OK))
		goto err;

	struct stat st;
	stat(path, &st);

	// if path is a directory -> obtain childs entries and put into buf
	if (S_ISDIR(st.st_mode))
	{
		DIR *dir = opendir(path);
		if (dir == NULL)
			goto err;
		struct dirent *entry;
		Entry *childs;
		uint countOfChilds = 0;
		while ((entry = readdir(dir)) != NULL)
		{
			if (countOfChilds == 0)
				childs = (Entry *)malloc((countOfChilds = 1) * sizeof(Entry));
			else
				childs = (Entry *)realloc(childs, (++countOfChilds) * sizeof(Entry));

			struct stat childStat;
			String childPath = strConcat(path, "/", entry->d_name);
			stat(childPath, &childStat);
			free(childPath);
			childs[countOfChilds - 1].name = strDup(entry->d_name);
			childs[countOfChilds - 1].parent = strDup(path);
			childs[countOfChilds - 1].fileSize = childStat.st_size;
			childs[countOfChilds - 1].dateModif = childStat.st_mtime;
			childs[countOfChilds - 1].isDir = S_ISDIR(childStat.st_mode);
			childs[countOfChilds - 1].isLink = S_ISLNK(childStat.st_mode);
			childs[countOfChilds - 1].permission = childStat.st_mode & 0x1FF;
		}
		closedir(dir);
		free(path);
		*buf = childs;
		return countOfChilds;
	}

	// if path is not a directory -> return 0 (it's exist)
	else
	{
		free(path);
		return 0;
	}

err:
	free(path);
	return -1;
}

int fileMemMove(FILE *file, long source, long destination, size_t size)
{
    char *buffer = malloc(size);
    if (buffer == NULL)
        return ERR_FILE_ERROR;

    // goto source
    fseek(file, source, SEEK_SET);

    // read from source
    fread(buffer, 1, size, file);

    // goto dest
    fseek(file, destination, SEEK_SET);

    // write in dest
    fwrite(buffer, 1, size, file);

    // free buffer
    free(buffer);

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

    return (removeLine(file, lineNumber) || insertLine(file, lineNumber, newContent));

    // long firstPos = ftell(file);
    // char buf[STR_LINE_MAX];
    // if (fgets(buf, STR_LINE_MAX - 1, file) == NULL)
    //     return ERR_FILE_ERROR;
    
    // fseek(file, 0, SEEK_END);
    // size_t size = ftell(file) - firstPos - strlen(buf);

    // // Move data within the file's memory
    // fileMemMove(file, firstPos + strlen(buf), firstPos + strlen(newContent), size);

    // fseek(file, firstPos, SEEK_SET);
    // fputs(newContent, file);

    // // Truncate the file to the adjusted size
    // ftruncate(fileno(file), size + firstPos + strlen(newContent));

    // if (ferror(file))
    //     return ERR_FILE_ERROR;
    // else
    //     return ERR_NOERR;
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


#ifdef _WIN32 // For Windows

#else // For Linux

#endif