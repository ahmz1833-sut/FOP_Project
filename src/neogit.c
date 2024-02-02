#include "neogit.h"

// Global variable for cwd (Used in other c files - valued in begining of main())
String curWorkingDir = NULL;
// Global variable for current repository (Used in other c files - valued in begining of main())
Repository *curRepository = NULL;

// root path must be absolute / list function must accept abs path and return abs path / relative path to repo passed to print function
int processTree(FileEntry *root, uint curDepth, int (*listFunction)(FileEntry **, constString), bool print_tree, void (*callbackFunction)(FileEntry *))
{
	uint processedEntries = 0;
	bool isLastBefore[16];

	if (print_tree)
	{
		// Extract is last before (in tree prinitng) from bits  of flags
		for (int i = 4; i < 20; i++)
			isLastBefore[i - 4] = curDepth & (1 << i);
		curDepth &= 0xF; // Clear all but lower 4 bits (real curDepth)
	}

	if (curDepth == 0) // base of recursive
		return 0;

	static int maxDepth = 0;
	if (maxDepth < curDepth) // First call
	{
		maxDepth = curDepth;
		// calculate relative path
		FileEntry fe = getFileEntry(root->path, curRepository->absPath);
		if (callbackFunction)
			callbackFunction(&fe);
		freeFileEntry(&fe, 1);
	}

	FileEntry *array;
	uint num = listFunction(&array, root->path);
	if (!num)
		return 0;

	for (int i = 0; i < num; i++)
	{
		if (print_tree)
		{
			for (int j = 0; j <= (maxDepth - curDepth); j++)
			{
				if (j != maxDepth - curDepth) // Indentations
					printf("%s   ", isLastBefore[j] ? " " : "│");
				else if (isLastBefore[maxDepth - curDepth] = (i == num - 1)) // Last entry of current directory
					printf("└── ");
				else
					printf("├── ");
			}
		}
		FileEntry relativeToRepo = getFileEntry(array[i].path, curRepository->absPath);
		if (callbackFunction)
			callbackFunction(&relativeToRepo);
		if (!relativeToRepo.isDir)
			processedEntries++;
		if (array[i].isDir && curDepth > 1 && !isMatch(relativeToRepo.path, "." PROGRAM_NAME))
		{
			uint passedInt = curDepth - 1;
			if (print_tree)
				for (int i = 4; i < 20; i++)
					passedInt |= (isLastBefore[i - 4] ? (1 << i) : 0);
			processedEntries += processTree(&array[i], passedInt, listFunction, print_tree, callbackFunction);
		}
		freeFileEntry(&relativeToRepo, 1);
	}
	freeFileEntry(array, num);
	free(array);
	return processedEntries;
}

int obtainRepository(constString workDir)
{
	// Declare variable to store the repository path
	String repoPath = NULL;

	// Push current working directory into temp variable
	withString(tmpWorkDir, getcwd(NULL, PATH_MAX))
	{
		// Change to the provided working directory
		chdir(workDir);

		// Iterate over the directory entries and search for the repository
		FileEntry *buf = NULL;
		int count = 0;
		bool found = false;
		while ((count = ls(&buf, ".")) > 0)
		{
			for (int i = 0; i < count; i++)
				if (buf[i].isDir && isMatch(getFileName(buf[i].path), "." PROGRAM_NAME))
				{
					repoPath = getcwd(NULL, PATH_MAX);
					found = true;
					break;
				}
			freeFileEntry(buf, count);
			free(buf);

			// If we found repo, or reach root, or error in navigating to parent, break loop;
			if (found || isMatch(getcwd(NULL, PATH_MAX), "/") || chdir("..") != 0)
				break;
		}

		// Change back to the original working directory (Pop from temp variable)
		chdir(tmpWorkDir);
	}
	if (repoPath)
	{
		curRepository = (Repository *)malloc(sizeof(Repository));
		curRepository->absPath = repoPath;
		curRepository->stagingArea.len = 0;
		curRepository->stagingArea.arr = NULL;
		curRepository->deatachedHead = false;
		curRepository->head.branch = NULL;
		curRepository->head.hash = 0xFFFFFF;
		curRepository->head.headFiles.arr = NULL;
		curRepository->head.headFiles.len = 0;

		fetchStagingArea();
		fetchHEAD();
		return ERR_NOERR;
	}
	else
		return ERR_NOREPO;
}

/**
 * @brief Get the configuration file path for a specified scope
 *
 * The ـgetconfigpath function generates the file path for the configuration file
 * based on the specified scope. If 'global' is true, it creates the global configuration
 * file in the user's home directory. Otherwise, it creates a repository-specific
 * configuration file in the current repository's directory. If the file does not exist,
 * it is created using the 'touch' command.
 *
 * Note: This function is not declared in any header file and is intended for internal use within the module.
 *
 * @param global If true, generate the path for the global configuration file; otherwise,
 *               generate the path for the repository-specific configuration file.
 *
 * @return A dynamically allocated string containing the absolute path to the configuration file.
 *         The caller is responsible for freeing the memory allocated for the string.
 */
String ـgetconfigpath(bool global)
{
	String configPath;

	// Check if the configuration should be global or repository-specific
	if (global)
	{
		// Generate the global configuration path in the user's home directory
		configPath = strConcat(getenv("HOME"), "/." PROGRAM_NAME "config");
		// Ensure the file exists by creating an empty file
		system("touch ~/." PROGRAM_NAME "config");
	}
	else
	{
		// Generate the repository-specific configuration path
		configPath = strConcat(curRepository->absPath, "/." PROGRAM_NAME "/config");
		char cmd[PATH_MAX];
		// Ensure the file exists by creating an empty file
		sprintf(cmd, "touch \"%s\"", configPath);
		system(cmd);
	}

	return configPath;
}

/**
 * @brief Load a configuration value and its associated timestamp from the specified configuration file.
 *
 * The _loadconfig function reads the configuration file to obtain the value and timestamp
 * associated with the given key. The configuration file is determined based on the 'global' parameter.
 * The obtained value is stored in the 'valueDest' buffer, and the timestamp is returned.
 *
 * Note: This function is not declared in any header file and is intended for internal use within the module.
 *
 * @param key The configuration key to retrieve its value and timestamp.
 * @param valueDest The buffer to store the retrieved value.
 * @param global If true, the global configuration file is used; otherwise, the
 *               repository-specific configuration file is used.
 *
 * @return The timestamp associated with the key-value pair in the configuration file.
 *         If the key is not found or an error occurs, 0 is returned.
 *         The retrieved value is stored in 'valueDest'.
 */
time_t _loadconfig(constString key, String valueDest, bool global)
{
	time_t configTime = 0;

	// Obtain the path of the configuration file and Open the configuration file for reading
	tryWithString(configPath, ـgetconfigpath(global), ({ return 0; }), ({ return 0; }))
	{
		tryWithFile(configFile, configPath, throw(0), throw(0))
		{
			// Prepare the format string for reading the configuration line
			String configFormat = strConcat("[", key, "]:%[^:]:%ld\n");

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strConcat("[", key, "]:*:*");

			// Read and parse the configuration file to find the specified key
			char buf[STR_LINE_MAX];
			uint lineNum;
			for (lineNum = 1; fgets(buf, STR_LINE_MAX - 1, configFile) != 0; lineNum++)
				if (isMatch(buf, configWildcard))
				{
					sscanf(buf, configFormat, valueDest, &configTime);
					break;
				}

			// Free allocated memory and close the configuration file
			free(configFormat);
			free(configWildcard);
		}
	}

	return configTime;
}

int setConfig(constString key, constString value, time_t now, bool global)
{
	// Obtain the path of the configuration file and Open the configuration file
	tryWithString(configPath, ـgetconfigpath(global), ({ return ERR_FILE_ERROR; }), __retTry)
	{
		tryWithFile(configFile, configPath, throw(ERR_FILE_ERROR), throw(_ERR))
		{
			// Prepare the format string and line content for the new configuration
			String configFormat = strConcat("[", key, "]:%s:%ld\n");
			String configLineString = malloc(STR_LINE_MAX);
			sprintf(configLineString, configFormat, value, now);

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strConcat("[", key, "]:*:*");

			// Search for the line number where the key is located in the configuration file
			char buf[STR_LINE_MAX];
			uint lineNum;
			for (lineNum = 1; fgets(buf, STR_LINE_MAX - 1, configFile) != 0; lineNum++)
				if (isMatch(buf, configWildcard))
					break;

			// Replace or insert the configuration line at the identified line number
			int res = replaceLine(configFile, lineNum, configLineString);

			// Free allocated memory
			free(configLineString);
			free(configFormat);
			free(configWildcard);

			throw(res);
		}
	}
	return ERR_NOERR;
}

String getConfig(constString key)
{
	char globalValue[STR_LINE_MAX] = {0};
	time_t globalConfigSetTime = _loadconfig(key, globalValue, true);

	if (!curRepository) // Repo not initialized
		return *globalValue ? strDup(globalValue) : NULL;

	char localValue[STR_LINE_MAX] = {0};
	time_t localConfigSetTime = _loadconfig(key, localValue, false);

	/* If the local configuration is set and it's newer than the global one, use
	the local version. Otherwise, use the global one */
	if (!localConfigSetTime || (globalConfigSetTime && localConfigSetTime < globalConfigSetTime))
		return *globalValue ? strDup(globalValue) : NULL;
	else
		return *localValue ? strDup(localValue) : NULL;
}

int removeConfig(constString key, bool global)
{
	// Obtain the path of the configuration file and Open the configuration file
	tryWithString(configPath, ـgetconfigpath(global), ({ return ERR_FILE_ERROR; }), __retTry)
	{
		tryWithFile(configFile, configPath, throw(ERR_FILE_ERROR), throw(_ERR))
		{
			// Prepare the format string for reading the configuration line
			String configFormat = strConcat("[", key, "]:%[^:]:%ld\n");

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strConcat("[", key, "]:*:*");

			// Read and parse the configuration file to find the specified key
			char buf[STR_LINE_MAX];
			uint lineNum;
			bool found = false;
			for (lineNum = 0; !found && (fgets(buf, STR_LINE_MAX - 1, configFile) != 0); lineNum++)
				if (isMatch(buf, configWildcard))
					found = true;

			int res = found ? removeLine(configFile, lineNum) : ERR_CONFIG_NOTFOUND;

			// Free allocated memory and close the configuration file
			free(configFormat);
			free(configWildcard);

			throw(res);
		}
	}
	return ERR_NOERR;
}

int getAliases(String *keysDest)
{
	uint aliasLen = 0;

	// The format for parsing alias configurations in the files
	String configFormat = strConcat("[alias.%[^]]]:%[^:]:");
	String configWildcard = strConcat("[alias.*]:*:*");
	char buf[STR_LINE_MAX];
	uint lineNum;

	// Retrieve alias keys from the global configuration
	tryWithString(GconfigPath, ـgetconfigpath(true), {}, {})
	{
		tryWithFile(GconfigFile, GconfigPath, {}, {})
		{
			for (lineNum = 1; fgets(buf, STR_LINE_MAX - 1, GconfigFile) != 0; lineNum++)
				if (isMatch(buf, configWildcard))
				{
					char keyBuf[STR_LINE_MAX], cmdBuf[STR_LINE_MAX];
					sscanf(buf, configFormat, keyBuf, cmdBuf);
					bool found = false;
					// Check for duplicate keys in the destination array
					for (int i = 0; !found && i < aliasLen; i++)
						if (strcmp(keyBuf, keysDest[i]) == 0)
							found = true;
					// If the key is not a duplicate, add it to the destination array
					if (!found)
						keysDest[aliasLen++] = strDup(keyBuf);
				}
		}
	}

	// Retrieve alias keys from the local configuration if in a repository
	if (curRepository)
	{
		tryWithString(LconfigPath, ـgetconfigpath(false), {}, {})
		{
			tryWithFile(LconfigFile, LconfigPath, {}, {})
			{
				for (lineNum = 1; LconfigFile && (fgets(buf, STR_LINE_MAX - 1, LconfigFile) != 0); lineNum++)
					if (isMatch(buf, configWildcard))
					{
						char keyBuf[STR_LINE_MAX], cmdBuf[STR_LINE_MAX];
						sscanf(buf, configFormat, keyBuf, cmdBuf);
						bool found = false;
						// Check for duplicate keys in the destination array
						for (int i = 0; !found && i < aliasLen; i++)
							if (strcmp(keyBuf, keysDest[i]) == 0)
								found = true;
						// If the key is not a duplicate, add it to the destination array
						if (!found)
							keysDest[aliasLen++] = strDup(keyBuf);
					}
			}
		}
	}
	return aliasLen;
}

bool isGitIgnore(FileEntry *entry)
{
	return entry->isDir && isMatch(getFileName(entry->path), ".git");
}

// the input path must be relative to the repo
StagedFile *getStagedFile(constString path)
{
	for (int i = 0; i < curRepository->stagingArea.len; i++)
		if (!strcmp(curRepository->stagingArea.arr[i].file.path, path))
			return &(curRepository->stagingArea.arr[i]);
	return NULL;
}

// the input path must be relative to the repo
StagedFile *getHEADFile(constString path)
{
	for (int i = 0; i < curRepository->head.headFiles.len; i++)
		if (!strcmp(curRepository->head.headFiles.arr[i].file.path, path))
			return &(curRepository->head.headFiles.arr[i]);
	return NULL;
}

// the input path must be relative to the repo
int removeFromStage(constString filePath)
{
	StagedFile *sf = NULL;
	if (!(sf = getStagedFile(filePath)))
		return ERR_NOT_EXIST;

	if (!sf->file.isDeleted) // We must delete the staged file
		withString(oldPath, strConcat(curRepository->absPath, "/." PROGRAM_NAME "/stage", sf->hashStr))
			remove(oldPath);

	// remove from info file
	char path[PATH_MAX];
	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info");
	tryWithFile(infoFile, path, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		int error = 0;
		char pattern[STR_LINE_MAX];
		sprintf(pattern, "%s:%ld:%u:%s", sf->file.path, sf->file.dateModif, sf->file.permission, sf->hashStr);
		int res = searchLine(infoFile, pattern);
		if (res != -1)
			error = removeLine(infoFile, res);
		else
			error = ERR_NOT_EXIST;
		throw(error);
	}

	// Refresh Structs in program
	fetchStagingArea(curRepository);

	return ERR_NOERR;
}

// the input path must be relative to the repo
int addToStage(constString filePath)
{
	StagedFile *sf = NULL;
	if (!(sf = getStagedFile(filePath)))
	{
		ADD_EMPTY(curRepository->stagingArea, StagedFile);
		sf = &(curRepository->stagingArea.arr[curRepository->stagingArea.len - 1]);
	}
	else if (!sf->file.isDeleted)
		withString(oldPath, strConcat(curRepository->absPath, "/." PROGRAM_NAME "/stage", sf->hashStr))
			remove(oldPath);

	withString(absPath, strConcat(curRepository->absPath, "/", filePath))
		sf->file = getFileEntry(absPath, curRepository->absPath);

	if (!(sf->file.isDeleted))
	{
		withString(hash, toHexString(generateUniqueId(10), 10))
			strcpy(sf->hashStr, hash);
		char hashedPath[PATH_MAX];
		strConcatStatic(hashedPath, "." PROGRAM_NAME "/stage/", sf->hashStr);
		int err = copyFile(sf->file.path, hashedPath, curRepository->absPath);
		if (err)
			return err;
	}
	else
		strcpy(sf->hashStr, "dddddddddd");

	// save information to info file
	char path[PATH_MAX];
	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info");
	systemf("mkdir -p \"%s/." PROGRAM_NAME "/stage\"", curRepository->absPath);
	systemf("touch \"%s\"", path);

	// add to info file
	tryWithFile(infoFile, path, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		int error = 0;
		char pattern[STR_LINE_MAX];
		sprintf(pattern, "%s:*:*:*", sf->file.path);
		int res = searchLine(infoFile, pattern);
		sprintf(pattern, "%s:%ld:%u:%s\n", sf->file.path, sf->file.dateModif, sf->file.permission, sf->hashStr);
		if (res != -1)
			error = replaceLine(infoFile, res, pattern);
		else
			fputs(pattern, freopen(path, "ab+", infoFile));
		throw(error);
	}
	return ERR_NOERR;
}

// the input path must be relative to the repo
int trackFile(constString filepath)
{
	if (isTrackedFile(filepath))
		return ERR_NOERR;
	char manifestFilePath[PATH_MAX];
	strConcatStatic(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
	systemf("touch \"%s\"", manifestFilePath);
	tryWithFile(manifestFile, manifestFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		freopen(manifestFilePath, "a+", manifestFile);
		fputs(filepath, manifestFile);
		fputc('\n', manifestFile);
	}
	return ERR_NOERR;
}

// the input path must be relative to the repo
bool isTrackedFile(constString path)
{
	char manifestFilePath[PATH_MAX];
	strConcatStatic(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
	systemf("touch \"%s\"", manifestFilePath);
	tryWithFile(manifestFile, manifestFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char buf[PATH_MAX];
		while (fgets(buf, sizeof(buf), manifestFile) != NULL)
			if (isMatch(buf, path))
				return true;
	}
}

// Dangerous function! always pay attention and note down what you are doing.
int applyToWorkingDir(StagedFileArray head)
{
	char manifestFilePath[PATH_MAX];
	strConcatStatic(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
	systemf("touch \"%s\"", manifestFilePath);
	tryWithFile(manifestFile, manifestFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		// Path of tracking file (relative to repo)
		char buf[PATH_MAX];
		// Iterate over tracked files
		while (fgets(buf, sizeof(buf), manifestFile) != NULL)
		{
			char absPath[PATH_MAX];
			strtrim(buf);
			strConcatStatic(absPath, curRepository->absPath, "/", buf);
			ChangeStatus state = getChangesFromHEAD(buf);
			switch (state)
			{
			case ADDED:			 // We have to remove this file from working tree
				remove(absPath); // DELETE THE FILE IN WORKING TREE !!
				break;
			case DELETED:  // We have to add this file to working tree
			case MODIFIED: // We have to update this file at working tree
				StagedFile *sf = getHEADFile(buf);
				char objAbsPath[PATH_MAX];
				strConcatStatic(objAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", sf->hashStr);
				copyFile(objAbsPath, absPath, ""); // REPLACE THE FILE IN WORKING TREE WITH HEAD ONE !!
				// TODO: set the file(absPath) timestamp to sf->file.dateModif
				struct utimbuf newTime;
				newTime.actime = time(NULL);		  // Access time set to now
				newTime.modtime = sf->file.dateModif; // Modification time is set to the original timestamp
				utime(absPath, &newTime);
				break;
			}
		}
	}
	return ERR_NOERR;
}

bool isWorkingTreeModified()
{
	char manifestFilePath[PATH_MAX];
	strConcatStatic(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
	systemf("touch \"%s\"", manifestFilePath);
	tryWithFile(manifestFile, manifestFilePath, ({ return false; }), __retTry)
	{
		// Path of tracking file (relative to repo)
		char buf[PATH_MAX];
		bool flag = false;
		// Iterate over tracked files
		while (fgets(buf, sizeof(buf), manifestFile) != NULL)
		{
			strtrim(buf);
			if (getChangesFromHEAD(buf))
			{
				flag = true;
				break;
			}
		}
		throw(flag);
	}
}

// Input paths : abs or rel to cwd
// paths in Output buf: absolute
int lsWithHead(FileEntry **buf, constString path)
{
	FileEntry *__buf;
	int __entry_count = ls(&__buf, path);
	if (__entry_count == -2)
		return -2; // if path is a file, and exist, return -2

	for (int i = 0; i < curRepository->head.headFiles.len; i++)
	{
		FileEntry *entry = &(curRepository->head.headFiles.arr[i].file);
		char gitEntryAbsPath[PATH_MAX], gitEntryAbsParent[PATH_MAX], inputAbsPath[PATH_MAX];
		strConcatStatic(gitEntryAbsPath, curRepository->absPath, "/", entry->path); // make abs path of entry
		getParentName(gitEntryAbsParent, gitEntryAbsPath);							// get entry's parent abs path
		withString(s, normalizePath(path, NULL))									// get input abs path
			strcpy(inputAbsPath, s);
		if (!isMatch(gitEntryAbsParent, inputAbsPath) && !isMatch(gitEntryAbsPath, inputAbsPath))
			continue;							// doesn't belong to requested path
		if (access(gitEntryAbsPath, F_OK) == 0) // real file exist!
			continue;							// already exist in __buf

		if (isMatch(gitEntryAbsPath, inputAbsPath))
			return -2; // It's File. Not folder!

		if (__entry_count <= 0)
			__buf = (FileEntry *)malloc((__entry_count = 1) * sizeof(FileEntry));
		else
			__buf = (FileEntry *)realloc(__buf, (++__entry_count) * sizeof(FileEntry));

		__buf[__entry_count - 1] = *entry;
		__buf[__entry_count - 1].isDeleted = true; // because it is not in the working tree now
	}
	if(__entry_count > 0) qsort(__buf, __entry_count, sizeof(FileEntry), __file_entry_sort_comparator);
	*buf = __buf;
	return __entry_count;
}

int lsChangedFiles(FileEntry **buf, constString dest)
{
	FileEntry *mybuf = NULL;
	int count = lsWithHead(&mybuf, dest);
	if (count <= 0)
		return count;

	int newCount = 0;
	for (int i = 0; i < count; i++)
	{
		FileEntry local = getFileEntry(mybuf[i].path, curRepository->absPath);
		if ((mybuf[i].isDir && lsChangedFiles(NULL, mybuf[i].path)) ||									// The entry is a dir has changed files
			(!mybuf[i].isDir && (getChangesFromHEAD(local.path) || getChangesFromStaging(local.path)))) // The entry is a changed file
		{
			// add to buf
			if (isGitIgnore(&local) || isMatch(local.path, ".neogit"))
				continue;
			if (buf)
			{
				if (newCount == 0)
					*buf = (FileEntry *)malloc((newCount = 1) * sizeof(FileEntry));
				else
					*buf = (FileEntry *)realloc(*buf, (++newCount) * sizeof(FileEntry));

				(*buf)[newCount - 1] = mybuf[i];
			}
			else
				newCount++;
		}
		freeFileEntry(&local, 1);
	}
	free(mybuf);
	return newCount;
}

// the input path must be relative to the repo
ChangeStatus getChangesFromHEAD(constString path)
{
	StagedFile *headFile = getHEADFile(path);

	char abspath[PATH_MAX];
	strConcatStatic(abspath, curRepository->absPath, "/", path);
	if (access(abspath, F_OK) != 0) // real file Not Exist
	{
		if (headFile == NULL) // this state can not occur (except calling with wrong path)
			return NOT_CHANGED;
		if (!(headFile->file.isDeleted)) // Th real file commited before!
			return DELETED;				 // but it's deleted from working dir
		else
			return NOT_CHANGED; // Deleted and Commited before!
	}

	if (headFile == NULL)
		return ADDED; // The File is Added after HEAD

	else // commited file and real file are present! compare them.
	{
		// absolute
		char headObjAbsPath[PATH_MAX];
		FileEntry realFile = getFileEntry(abspath, NULL);
		freeFileEntry(&realFile, 1);
		strConcatStatic(headObjAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", headFile->hashStr);
		if (headFile->file.isDeleted)
			return ADDED;
		else if (!isSameFiles(abspath, headObjAbsPath))
			return MODIFIED;
		else if (realFile.permission != headFile->file.permission)
			return PERM_CHANGED;
		else
			return NOT_CHANGED;
	}
}

// the input path must be relative to the repo
ChangeStatus getChangesFromStaging(constString path)
{
	StagedFile *stage = getStagedFile(path);
	char abspath[PATH_MAX];
	strConcatStatic(abspath, curRepository->absPath, "/", path);
	if (access(abspath, F_OK) != 0) // File Not Exist
	{
		if (stage == NULL)
			return getChangesFromHEAD(path);
		if (!(stage->file.isDeleted)) // Th real file staged before!
			return DELETED;			  // but it's deleted from working dir
		else
			return NOT_CHANGED; // Deleted and Staged before!
	}

	// real File is exist!

	if (!isTrackedFile(path)) // Untracked File
		return ADDED;
	else if (stage == NULL)
		return getChangesFromHEAD(path);
	else // staged file and real file are present! compare them.
	{
		// absolute
		char stagedObjAbsPath[PATH_MAX];
		FileEntry realFile = getFileEntry(abspath, NULL);
		freeFileEntry(&realFile, 1);
		strConcatStatic(stagedObjAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/stage/", stage->hashStr);
		if (stage->file.isDeleted)
			return ADDED;
		else if (!isSameFiles(abspath, stagedObjAbsPath))
			return MODIFIED;
		else if (realFile.permission != stage->file.permission)
			return PERM_CHANGED;
		else
			return NOT_CHANGED;
	}
}

int setBranchHead(constString branchName, uint64_t commitHash)
{
	char path[PATH_MAX];
	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");
	systemf("touch \"%s\"", path);
	tryWithFile(branchesFile, path, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char content[STR_LINE_MAX];
		sprintf(content, "%s:*", branchName);
		int result = searchLine(branchesFile, content);
		sprintf(content, "%s:%06lx\n", branchName, commitHash);
		if (result == -1)
			result = !fputs(content, freopen(path, "a+", branchesFile));
		else
			result = replaceLine(branchesFile, result, content);
		throw(result);
	}
	return ERR_NOERR;
}

uint64_t getBranchHead(constString branchName)
{
	char path[PATH_MAX];
	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");
	systemf("touch \"%s\"", path);
	tryWithFile(branchesFile, path, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char pattern[STR_LINE_MAX];
		sprintf(pattern, "%s:*", branchName);
		int result = searchLine(branchesFile, pattern);
		if (result == -1)
			throw(0x1FFFFFF); // branch not exist
		else
		{
			uint64_t hash = 0;
			SEEK_TO_LINE(branchesFile, result);
			strConcatStatic(pattern, branchName, ":%lx");
			fscanf(branchesFile, pattern, &hash);
			if (hash == 0)
				hash = 0xFFFFFF;
			throw(hash);
		}
	}
	return 0;
}

uint64_t getBrachHeadPrev(constString branchName, uint order)
{
	uint64_t curHash = getBranchHead(branchName);
	for (int i = 0; i < order; i++)
	{
		Commit *commit = getCommit(curHash);
		if (commit == NULL)
		{
			curHash = 0xFFFFFF;
			break;
		}
		curHash = commit->prev;
	}
	return curHash;
}

int listBranches(String *namesDest, uint64_t *hashesDest)
{
	int brCount = -1;
	char path[PATH_MAX];
	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");
	systemf("touch \"%s\"", path);
	tryWithFile(branchesFile, path, ({ return -1; }), __retTry)
	{
		do
			namesDest[++brCount] = malloc(STR_LINE_MAX);
		while (fscanf(branchesFile, "%[^:]:%lx\n", namesDest[brCount], hashesDest + brCount) != EOF);
		throw(brCount);
	}
}

// fromWhere : e.g. : "stage"
Commit *createCommit(StagedFileArray *filesToCommit, String fromWhere, String username, String email, String message)
{
	if (curRepository->deatachedHead)
		return NULL;

	HEAD *head = &(curRepository->head);
	Commit *newCommit = malloc(sizeof(Commit));
	newCommit->hash = generateUniqueId(6);
	newCommit->prev = head->hash;
	newCommit->branch = head->branch;
	newCommit->message = strDup(message);
	newCommit->username = strDup(username);
	newCommit->useremail = strDup(email);
	newCommit->time = time(NULL);
	newCommit->mergedCommit = 0;

	// Update head files
	for (int i = 0; i < filesToCommit->len; i++)
	{
		StagedFile *sf = &(filesToCommit->arr[i]);
		bool found = false;
		for (int j = 0; j < curRepository->head.headFiles.len; j++)
		{
			if (!strcmp(curRepository->head.headFiles.arr[j].file.path, sf->file.path))
			{
				found = true;
				curRepository->head.headFiles.arr[j] = *sf;
			}
		}
		if (!found)
		{
			ADD_EMPTY(curRepository->head.headFiles, StagedFile);
			curRepository->head.headFiles.arr[curRepository->head.headFiles.len - 1] = *sf;
		}
	}

	newCommit->commitedFiles = *filesToCommit;
	newCommit->headFiles = curRepository->head.headFiles;

	// copy objects from (fromWhere) to .neogit/objects
	for (int i = 0; i < filesToCommit->len; i++)
	{
		char p1[PATH_MAX], p2[PATH_MAX];
		copyFile(strConcatStatic(p1, "." PROGRAM_NAME "/", fromWhere, "/", filesToCommit->arr[i].hashStr),
				 strConcatStatic(p2, "." PROGRAM_NAME "/objects/", filesToCommit->arr[i].hashStr), curRepository->absPath);
	}

	char commitPath[PATH_MAX];
	sprintf(commitPath, "%s/." PROGRAM_NAME "/commits/%06lx", curRepository->absPath, newCommit->hash);
	systemf("mkdir -p \"%s/." PROGRAM_NAME "/commits\"", curRepository->absPath);
	systemf("touch \"%s\"", commitPath);
	tryWithFile(commitFile, commitPath, ({ return NULL; }), ({ return NULL; }))
	{
		// Commit File Structure:
		// Line 1 : "<username>:<email>:<time>:<branch>"
		// Line 2 : "[perv]:<pervHash>" or  "[perv]:<pervHash>:[merged]:<mergedHash>"
		// Line 3 : "[message]:[<message>]"
		// Line 4 : "[<a>]:[<b>]"   // a : number of commit files , b : number of head files
		// Line 5 -> 4+a : Commited files (same format with staging info)
		// Line 5+a : \n
		// Line 6+a -> 5+a+b : HEAD files (same format with staging info)
		fprintf(commitFile, "%s:%s:%ld:%s\n", newCommit->username, newCommit->useremail, newCommit->time, newCommit->branch);
		fprintf(commitFile, "[perv]:%06lx\n", newCommit->prev);
		fprintf(commitFile, "[message]:[%s]\n", message);
		fprintf(commitFile, "[%u]:[%u]\n", newCommit->commitedFiles.len, newCommit->headFiles.len);
		for (int i = 0; i < newCommit->commitedFiles.len; i++)
		{
			StagedFile *sf = &(newCommit->commitedFiles.arr[i]);
			// <path>:<timeModif>:<perm>:<10digitHash>
			fprintf(commitFile, "%s:%ld:%u:%s\n", sf->file.path, sf->file.dateModif, sf->file.permission, sf->hashStr);
		}
		fputs("\n", commitFile);
		for (int i = 0; i < newCommit->headFiles.len; i++)
		{
			StagedFile *sf = &(newCommit->headFiles.arr[i]);
			// <path>:<timeModif>:<perm>:<10digitHash>
			fprintf(commitFile, "%s:%ld:%u:%s\n", sf->file.path, sf->file.dateModif, sf->file.permission, sf->hashStr);
		}
	}

	// Update head hash
	curRepository->head.hash = newCommit->hash;
	// Update current branch head
	setBranchHead(newCommit->branch, newCommit->hash);

	return newCommit;
}

Commit *getCommit(uint64_t hash)
{
	Commit *dynamic_allocated_commit = NULL;
	char commitPath[PATH_MAX];
	sprintf(commitPath, "%s/." PROGRAM_NAME "/commits/%06lx", curRepository->absPath, hash);
	tryWithFile(commitFile, commitPath, ({ return NULL; }), ({ return NULL; }))
	{
		char name[STR_MAX] = {0}, email[STR_MAX] = {0}, branch[STR_MAX] = {0};
		char message[COMMIT_MSG_LEN_MAX] = {0};
		Commit commit = {0, NULL, NULL, 0, NULL, NULL, {NULL, 0}, {NULL, 0}, 0, 0};
		// Commit File Structure:
		// Line 1 : "<username>:<email>:<time>:<branch>"
		// Line 2 : "[perv]:<pervHash>" or  "[perv]:<pervHash>:[merged]:<mergedHash>"
		// Line 3 : "[message]:[<message>]"
		// Line 4 : "[<a>]:[<b>]"   // a : number of commit files , b : number of head files
		// Line 5 -> 4+a : Commited files (same format with staging info)
		// Line 5+a : \n
		// Line 6+a -> 5+a+b : HEAD files (same format with staging info)
		fscanf(commitFile, "%[^:]:%[^:]:%ld:%[^\n]\n", name, email, &(commit.time), branch);
		if (!*name || !*email || !commit.time || !*branch)
			throw(0);
		fscanf(commitFile, "[perv]:%lx\n", &(commit.prev));
		if (!commit.prev)
			throw(0);
		fscanf(commitFile, "[message]:[%[^]]]\n", message);
		if (!*message)
			throw(0);
		fscanf(commitFile, "[%u]:[%u]\n", &(commit.commitedFiles.len), &(commit.headFiles.len));
		if (!commit.commitedFiles.len || !commit.headFiles.len)
			throw(0);

		commit.hash = hash;
		commit.username = strDup(name);
		commit.useremail = strDup(email);
		commit.branch = strDup(branch);
		commit.message = strDup(message);

		commit.commitedFiles.arr = malloc(sizeof(StagedFile) * commit.commitedFiles.len);
		commit.headFiles.arr = malloc(sizeof(StagedFile) * commit.headFiles.len);

		for (int i = 0; i < commit.commitedFiles.len; i++)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			fscanf(commitFile, "%[^:]:%ld:%u:%s\n", filePath, &timeM, &perm, hash);

			StagedFile *sf = &(commit.commitedFiles.arr[i]);
			sf->file.path = strDup(filePath);
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.permission = perm;
		}

		for (int i = 0; i < commit.headFiles.len; i++)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			fscanf(commitFile, "\n%[^:]:%ld:%u:%s\n", filePath, &timeM, &perm, hash);

			StagedFile *sf = &(commit.headFiles.arr[i]);
			sf->file.path = strDup(filePath);
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.permission = perm;
		}
		if (ferror(commitFile))
			throw(0);

		dynamic_allocated_commit = malloc(sizeof(Commit));
		*dynamic_allocated_commit = commit;
	}
	return dynamic_allocated_commit;
}

void freeCommitStruct(Commit *object)
{
	if (object)
	{
		if (object->branch)
			free(object->branch);
		if (object->username)
			free(object->username);
		if (object->useremail)
			free(object->useremail);
		if (object->message)
			free(object->message);
		if (object->headFiles.arr)
			free(object->headFiles.arr);
		if (object->commitedFiles.arr)
			free(object->commitedFiles.arr);
		free(object);
	}
}

int restoreStageingBackup()
{
	char tmp[PATH_MAX];
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old0", curRepository->absPath);
	if (access(tmp, F_OK) == -1)
		return ERR_NOT_EXIST; // check if not exist

	FileEntry *buf = NULL;
	strConcatStatic(tmp, curRepository->absPath, "/." PROGRAM_NAME "/stage");
	int res = ls(&buf, tmp);
	int error = 0;
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			remove(buf[i].path);
	freeFileEntry(buf, res);

	// remove the top stack
	char stagePath[PATH_MAX];
	strcpy(stagePath, tmp);
	strcat(tmp, "/old0");
	systemf("mv \"%s\"/* \"%s\" 2>/dev/null", tmp, stagePath); // move back all backed up files
	systemf("rm -r \"%s\"", tmp);							   // remove backup folder

	for (int i = 1; i <= 9; i++)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);
		// check if not exist
		if (access(tmp, F_OK) == -1)
			break;

		// move top
		tmp[strlen(tmp) - 1] = '\0'; // truncate and remove number from end of filename
		systemf("mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i - 1);
	}
	return ERR_NOERR;
}

int backupStagingArea()
{
	char tmp[PATH_MAX];
	for (int i = 9; i >= 0; i--)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);
		// check if exist
		if (access(tmp, F_OK) == -1)
			continue;

		// move to newer one
		char cmd[PATH_MAX * 2 + 10];
		tmp[strlen(tmp) - 1] = '\0'; // truncate and remove number from end of filename
		if (i == 9)
			sprintf(cmd, "rm -r \"%s%d\"", tmp, i);
		else
			sprintf(cmd, "mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i + 1);

		system(cmd);
	}
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage", curRepository->absPath);
	FileEntry *buf = NULL;
	int res = ls(&buf, tmp);
	int error = 0;
	strcat(tmp, "/old0");
	mkdir(tmp, 0775);
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			withString(dest, strConcat(tmp, "/", getFileName(buf[i].path)))
				error += copyFile(buf[i].path, dest, "");
	freeFileEntry(buf, res);
	return error;
}

int fetchStagingArea()
{
	char path[PATH_MAX];

	if (curRepository->stagingArea.arr)
	{
		free(curRepository->stagingArea.arr);
		curRepository->stagingArea.arr = NULL;
		curRepository->stagingArea.len = 0;
	}

	tryWithFile(infoFile, strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info"),
				({ return ERR_FILE_ERROR; }), __retTry)
	{
		char buf[STR_LINE_MAX];
		int error = 0;
		while (fgets(buf, STR_LINE_MAX, infoFile) != NULL)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			sscanf(buf, "%[^:]:%ld:%u:%s", filePath, &timeM, &perm, hash);

			StagedFile *sf = getStagedFile(filePath);
			if (sf == NULL)
			{
				ADD_EMPTY(curRepository->stagingArea, StagedFile);
				sf = &(curRepository->stagingArea.arr[curRepository->stagingArea.len - 1]);
				sf->file.path = strDup(filePath);
			}
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.permission = perm;
		}
		rewind(infoFile);
	}
}

int fetchHEAD()
{
	char path[PATH_MAX];
	curRepository->deatachedHead = false;
	curRepository->head.branch = "master";
	curRepository->head.hash = 0xFFFFFF;
	curRepository->head.headFiles.arr = NULL;
	curRepository->head.headFiles.len = 0;

	strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/HEAD");
	systemf("touch \"%s\"", path);

	char HEAD_content[STR_LINE_MAX] = {0};
	tryWithFile(HEADfile, path, ({ return ERR_FILE_ERROR; }), __retTry)
		fgets(HEAD_content, STR_LINE_MAX, HEADfile);

	Commit *head = NULL;
	if (isMatch(HEAD_content, "branch/*"))
	{
		char branch[STR_LINE_MAX];
		sscanf(HEAD_content, "branch/%s", branch);
		curRepository->head.branch = strDup(branch);
		curRepository->head.hash = getBranchHead(branch) & 0xFFFFFF;
		head = getCommit(curRepository->head.hash);
	}
	else if (isMatch(HEAD_content, "commit/*"))
	{
		curRepository->deatachedHead = true;
		sscanf(HEAD_content, "commit/%lx", &curRepository->head.hash);
		head = getCommit(curRepository->head.hash);
		if (head)
			curRepository->head.branch = head->branch;
	}
	else
		systemf("echo branch/master>\"%s/." PROGRAM_NAME "/HEAD\"", curRepository->absPath);

	if (head)
	{
		// obtain head files
		curRepository->head.headFiles = head->headFiles;

		if (head->username)
			free(head->username);
		if (head->useremail)
			free(head->useremail);
		if (head->message)
			free(head->message);
		if (head->commitedFiles.arr)
			free(head->commitedFiles.arr);
		// We don't free "branch" and "headFiles arr" (we need them)
		free(head);
	}
	return ERR_NOERR;
}