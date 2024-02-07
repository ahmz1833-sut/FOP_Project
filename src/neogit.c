/*******************************
 *         neogit.c            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "neogit.h"

// Global variable for cwd (Used in other c files - valued in begining of main())
String curWorkingDir = NULL;
// Global variable for current repository (Used in other c files - valued in obtainRepository())
Repository *curRepository = NULL;

/////////////////////////// GENERAL FUNCTIONS + CONFIG and ALIAS ////////////////////////

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
		while (!found)
		{
			// iterate  over all file entries to find .neogit folder
			count = ls(&buf, ".");
			for (int i = 0; i < count; i++)
				if (buf[i].isDir && isMatch(getFileName(buf[i].path), "." PROGRAM_NAME))
				{
					repoPath = getcwd(NULL, PATH_MAX);
					found = true;
					break;
				}
			freeFileEntry(buf, count);
			free(buf);

			char cwd[PATH_MAX];

			// If we reach root, or error in navigating to parent, break loop;
			if (isMatch(getcwd(cwd, PATH_MAX), "/") || chdir("..") != 0)
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
		configPath = strcat_d(getenv("HOME"), "/." PROGRAM_NAME "config");
		// Ensure the file exists by creating an empty file
		system("touch ~/." PROGRAM_NAME "config");
	}
	else
	{
		// Generate the repository-specific configuration path
		configPath = strcat_d(curRepository->absPath, "/." PROGRAM_NAME "/config");
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
			String configFormat = strcat_d("[", key, "]:%[^:]:%ld\n");

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strcat_d("[", key, "]:*:*");
			
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
			String configFormat = strcat_d("[", key, "]:%s:%ld\n");
			String configLineString = malloc(STR_LINE_MAX);
			sprintf(configLineString, configFormat, value, now);

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strcat_d("[", key, "]:*:*");

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
			String configFormat = strcat_d("[", key, "]:%[^:]:%ld\n");

			// Create a wildcard pattern to match the key in the configuration file
			String configWildcard = strcat_d("[", key, "]:*:*");

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
	String configFormat = strcat_d("[alias.%[^]]]:%[^:]:");
	String configWildcard = strcat_d("[alias.*]:*:*");
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

///////////////////// FUNCTIONS RELATED TO STAGING AREA / ADD / Status ////////////////////////

bool isGitIgnore(FileEntry *entry)
{
	return entry->isDir && isMatch(getFileName(entry->path), ".git");
}

int trackFile(constString filepath)
{
	// Check if the file is already tracked
	if (isTrackedFile(filepath))
		return ERR_NOERR;

	// Construct the path to the manifest file
	char manifestFilePath[PATH_MAX];
	strcat_s(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");

	// Ensure the manifest file exists
	systemf("touch \"%s\"", manifestFilePath);

	// Open the manifest file for appending
	tryWithFile(manifestFile, manifestFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		// Append the file path to the manifest file
		freopen(manifestFilePath, "a+", manifestFile);
		fputs(filepath, manifestFile);
		fputc('\n', manifestFile);
	}
	return ERR_NOERR;
}

bool isTrackedFile(constString path)
{
	// Construct the path to the manifest file
	char manifestFilePath[PATH_MAX];
	strcat_s(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");

	// Ensure the manifest file exists
	systemf("touch \"%s\"", manifestFilePath);

	// Open the manifest file for reading
	tryWithFile(manifestFile, manifestFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char buf[PATH_MAX];
		bool found = false;
		// Iterate through the lines in the manifest file
		while (fgets(buf, sizeof(buf), manifestFile) != NULL)
			// Check if the file path matches the current line in the manifest
			if (isMatch(buf, path))
			{
				found = true;
				break;
			}

		throw(found);
	}
	return false;
}

ullong generateUniqueId(int hexdigits)
{
	// Get the current time
	time_t currentTime = time(NULL);
	clock_t _clock = clock();

	// Convert the time to a string
	char timestr[20];
	sprintf(timestr, "%ld%ld", currentTime, _clock);
	String str = timestr;

	// Initialize the hash with a random value
	ullong hash = rand() * rand();

	// Iterate through the characters in the time string
	int c;
	while (c = *(str++))
		// Update the hash using the current character and random values
		hash = (((hash << (c % 16))) + hash) + (rand() % c);

	// If the desired length is less than 16, mask the hash to the appropriate length
	if (hexdigits < 16)
		hash &= (1ULL << (hexdigits * 4)) - 1;

	return hash;
}

int addToStage(constString filePath)
{
	// Declare a pointer to StagedFile
	GitObject *sf = NULL;
	// Check if the file is already in the staging area
	if (!(sf = getStagedFile(filePath)))
	{
		// If not, allocate memory for a new GitObject in the staging area
		ADD_EMPTY(curRepository->stagingArea.arr, curRepository->stagingArea.len, GitObject);
		sf = &(curRepository->stagingArea.arr[curRepository->stagingArea.len - 1]);
	}
	else if (!sf->file.isDeleted)
		// If the file is not deleted, remove the old file from the staging area
		withString(oldPath, strcat_d(curRepository->absPath, "/." PROGRAM_NAME "/stage", sf->hashStr))
			remove(oldPath);

	// Get the absolute path of the file
	withString(absPath, strcat_d(curRepository->absPath, "/", filePath))
		sf->file = getFileEntry(absPath, curRepository->absPath);

	if (!(sf->file.isDeleted))
	{
		// Generate a unique hash for the file and update GitObject information
		withString(hash, toHexString(generateUniqueId(10), 10))
			strcpy(sf->hashStr, hash);

		// Copy the file to the hashed path in the staging area
		char hashedPath[PATH_MAX];
		strcat_s(hashedPath, "." PROGRAM_NAME "/stage/", sf->hashStr);
		int err = copyFile(sf->file.path, hashedPath, curRepository->absPath);
		if (err)
			return err;
	}
	else // If the file is deleted, set a default hash value
		strcpy(sf->hashStr, "dddddddddd");

	// Save information to the info file
	char path[PATH_MAX];
	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info");
	systemf("mkdir -p \"%s/." PROGRAM_NAME "/stage\"", curRepository->absPath);
	systemf("touch \"%s\"", path);

	// Add or update information in the info file
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

int removeFromStage(constString filePath)
{
	// Declare a pointer to StagedFile
	GitObject *sf = NULL;

	// Check if the file is in the staging area
	if (!(sf = getStagedFile(filePath)))
		return ERR_NOT_EXIST;

	// Remove the staged file if it is not deleted
	if (!sf->file.isDeleted) // We must delete the staged file
		withString(oldPath, strcat_d(curRepository->absPath, "/." PROGRAM_NAME "/stage", sf->hashStr))
			remove(oldPath);

	// Remove information from the info file
	char path[PATH_MAX];
	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info");
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

	// Refresh GitObjectArray in the program
	fetchStagingArea(curRepository);

	return ERR_NOERR;
}

int backupStagingArea()
{
	char tmp[PATH_MAX];

	// Iterate through existing backups and move them to newer versions
	for (int i = 9; i >= 0; i--)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);
		// Check if the backup directory exists
		if (access(tmp, F_OK) == -1)
			continue;

		// Move to a newer version or remove if it's the last one
		tmp[strlen(tmp) - 1] = '\0'; // Truncate and remove the number from the end of the filename
		if (i == 9)
			systemf("rm -r \"%s%d\"", tmp, i);
		else
			systemf("mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i + 1);
	}

	// Create a new backup directory (old0) and copy the current staging area to it
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage", curRepository->absPath);
	FileEntry *buf = NULL;
	int res = ls(&buf, tmp);
	int error = 0;
	strcat(tmp, "/old0");
	mkdir(tmp, 0775);

	// Copy each file from the staging area to the backup directory
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			withString(dest, strcat_d(tmp, "/", getFileName(buf[i].path)))
				error += copyFile(buf[i].path, dest, NULL);

	// Free the memory used by the list of files
	freeFileEntry(buf, res);
	if (buf && res > 0)
		free(buf);
	return error;
}

int restoreStageingBackup()
{
	char tmp[PATH_MAX];
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old0", curRepository->absPath);

	// Check if the backup directory (old0) exists
	if (access(tmp, F_OK) == -1)
		return ERR_NOT_EXIST; // check if not exist

	FileEntry *buf = NULL;
	strcat_s(tmp, curRepository->absPath, "/." PROGRAM_NAME "/stage");
	int res = ls(&buf, tmp);

	// Remove existing files in the staging area
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			remove(buf[i].path);

	// Free the memory used by the list of files
	freeFileEntry(buf, res);
	if (buf)
		free(buf);

	// Move backed up files from the backup directory (old0) to the staging area
	char stagePath[PATH_MAX];
	strcpy(stagePath, tmp);
	strcat(tmp, "/old0");
	systemf("mv \"%s\"/* \"%s\" 2>/dev/null", tmp, stagePath); // move back all backed up files
	systemf("rm -r \"%s\"", tmp);							   // remove backup folder

	// Shift other backups to fill the gap
	for (int i = 1; i <= 9; i++)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);

		// Check if  backup directory not exists
		if (access(tmp, F_OK) == -1)
			break;

		// Move the top backup to the previous version
		tmp[strlen(tmp) - 1] = '\0'; // truncate and remove number from end of filename
		systemf("mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i - 1);
	}
	return ERR_NOERR;
}

GitObject *getStagedFile(constString path)
{
	for (int i = 0; i < curRepository->stagingArea.len; i++)
		if (!strcmp(curRepository->stagingArea.arr[i].file.path, path))
			return &(curRepository->stagingArea.arr[i]);
	return NULL;
}

int fetchStagingArea()
{
	// Declare a variable to store the file path
	char path[PATH_MAX];

	// Free the existing GitObjectArray in the current repository
	if (curRepository->stagingArea.arr)
	{
		free(curRepository->stagingArea.arr);
		curRepository->stagingArea.arr = NULL;
		curRepository->stagingArea.len = 0;
	}

	// Open the info file for reading
	tryWithFile(infoFile, strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info"),
				({ return ERR_FILE_ERROR; }), __retTry)
	{
		char buf[STR_LINE_MAX];
		// Read the information from the info file
		while (fgets(buf, STR_LINE_MAX, infoFile) != NULL)
		{
			// Declare variables to store file information
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];

			// Parse the information from the buffer
			sscanf(buf, "%[^:]:%ld:%u:%s", filePath, &timeM, &perm, hash);

			// Get the StagedFile corresponding to the file path
			GitObject *sf = getStagedFile(filePath);

			// If StagedFile does not exist, add it to the GitObjectArray
			if (sf == NULL)
			{
				ADD_EMPTY(curRepository->stagingArea.arr, curRepository->stagingArea.len, GitObject);
				sf = &(curRepository->stagingArea.arr[curRepository->stagingArea.len - 1]);
				sf->file.path = strDup(filePath);
			}

			// Update the StagedFile information
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.permission = perm;
		}
		rewind(infoFile);
	}
	return ERR_NOERR;
}

ChangeStatus getChangesFromStaging(constString path)
{
	// Get the staged file entry
	GitObject *stage = getStagedFile(path);

	char abspath[PATH_MAX];
	strcat_s(abspath, curRepository->absPath, "/", path);

	// Check if the file does not exist in the working directory
	if (access(abspath, F_OK) != 0)
	{
		// File does not exist in the working directory

		// Check if the file not present in staging area
		if (stage == NULL)
			return getChangesFromHEAD(path, curRepository->head.headFiles);

		// Check if the staged file is not marked as deleted
		if (!(stage->file.isDeleted)) // Th real file staged before!
			return DELETED;			  // but it's deleted from working dir
		else
			return NOT_CHANGED; // Deleted and Staged before!
	}

	// Real file exists in the working directory

	// Check if the file is untracked
	if (!isTrackedFile(path))
		return ADDED;
	else if (stage == NULL)
		return getChangesFromHEAD(path, curRepository->head.headFiles);
	else
	{
		// Both staged file and real file are present, compare them

		// Get the absolute path of the staged object
		char stagedObjAbsPath[PATH_MAX];
		FileEntry realFile = getFileEntry(abspath, NULL);
		freeFileEntry(&realFile, 1);
		strcat_s(stagedObjAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/stage/", stage->hashStr);

		// Check if the staged file is marked as deleted
		if (stage->file.isDeleted)
			return ADDED;

		// Check if the content of the real file and staged file are different
		else if (!isFilesSame(abspath, stagedObjAbsPath))
			return MODIFIED;

		// Check if the file permissions are different
		else if (realFile.permission != stage->file.permission)
			return PERM_CHANGED;

		else
			return NOT_CHANGED;
	}
}

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

	FileEntry *array = NULL;
	int num = listFunction(&array, root->path);
	if (num <= 0)
		return 0;

	for (int i = 0; i < num; i++)
	{
		if (print_tree)
		{
			for (int j = 0; j <= (maxDepth - curDepth); j++)
			{
				if (j != maxDepth - curDepth) // Indentations
					printf("%s   ", isLastBefore[j] ? " " : "│");
				else if ((isLastBefore[maxDepth - curDepth] = (i == num - 1))) // Last entry of current directory
					printf("└── ");
				else
					printf("├── ");
			}
		}
		FileEntry relativeToRepo = array[i];
		relativeToRepo.path = normalizePath(array[i].path, curRepository->absPath);
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
	if (num)
		free(array);
	return processedEntries;
}

int lsWithHead(FileEntry **buf, constString path)
{
	FileEntry *__buf;
	int __entry_count = ls(&__buf, path);

	// Check if the specified path is a file and exists
	if (__entry_count == -2)
		return -2;

	// Iterate over files in the HEAD commit , check if that are not in the working directory
	for (int i = 0; i < curRepository->head.headFiles.len; i++)
	{
		FileEntry *entry = &(curRepository->head.headFiles.arr[i].file);
		char gitEntryAbsPath[PATH_MAX], gitEntryAbsParent[PATH_MAX], inputAbsPath[PATH_MAX];

		// Get the absolute path of the file in the HEAD commit
		strcat_s(gitEntryAbsPath, curRepository->absPath, "/", entry->path);
		// Get the absolute parent path of the file in the HEAD commit
		getParentName(gitEntryAbsParent, gitEntryAbsPath);
		// Get the absolute path of the input directory
		withString(s, normalizePath(path, NULL))
			strcpy(inputAbsPath, s);

		// Check if the file already exists in the working directory
		if (access(gitEntryAbsPath, F_OK) == 0)
			continue; // Already exists in __buf

		// Check if the file is a directory
		if (isMatch(gitEntryAbsPath, inputAbsPath))
			return -2; // It's File. Not folder!

		// Check if the file belongs to the requested path
		if (!isMatch(gitEntryAbsParent, inputAbsPath))
		{
			// it's probable that this file belongs to subchilds of this root! (a deleted folder!!)
			String s = normalizePath(gitEntryAbsPath, inputAbsPath);

			if (s != NULL) // Yes!! it's under a deleted folder!
			{
				// if the entry has been added  before, don't add again
				bool found = false;
				for (int i = 0; i < __entry_count; i++)
				{
					if (!strcmp(getFileName(__buf[i].path), strtok(s, "/")))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					// Add that deleted folder to the list
					ADD_EMPTY(__buf, __entry_count, FileEntry);
					__buf[__entry_count - 1].isDeleted = 1;
					__buf[__entry_count - 1].isDir = 1;
					__buf[__entry_count - 1].path = strcat_d(inputAbsPath, "/", strtok(s, "/"));
					__buf[__entry_count - 1].permission = 0777;
				}
				free(s);
			}
			continue;
		}

		// Add the entry to the list
		ADD_EMPTY(__buf, __entry_count, FileEntry);
		__buf[__entry_count - 1] = *entry;
		__buf[__entry_count - 1].path = strDup(gitEntryAbsPath);
		__buf[__entry_count - 1].isDeleted = true; // because it is not in the working tree now
	}
	*buf = __buf;
	return __entry_count;
}

bool _ls_head_changed_files = true;
int lsChangedFiles(FileEntry **buf, constString dest)
{
	FileEntry *mybuf = NULL;
	int count = lsWithHead(&mybuf, dest);

	// Check if there are no entries in the destination directory
	if (count <= 0)
		return count;

	// Iterate over entries in the destination directory
	int newCount = 0;
	for (int i = 0; i < count; i++)
	{
		FileEntry local = getFileEntry(mybuf[i].path, curRepository->absPath);
		// Check if the entry is a directory with changed files or a changed file
		if (mybuf[i].isDir || getChangesFromStaging(local.path) || (_ls_head_changed_files && getChangesFromHEAD(local.path, curRepository->head.headFiles))) // The entry is a changed file
		{
			// Check if the entry is ignored or .neogit folder
			if (isGitIgnore(&local) || isMatch(local.path, "." PROGRAM_NAME))
				goto _continue;

			// continue if directory doesn't have any changed entry
			if (mybuf[i].isDir && !lsChangedFiles(NULL, mybuf[i].path))
				goto _continue;

			// Check if the buffer is provided, and add the entry to the buffer
			if (buf)
			{
				ADD_EMPTY(*buf, newCount, FileEntry);
				(*buf)[newCount - 1] = mybuf[i];
				((*buf)[newCount - 1]).path = strDup(mybuf[i].path);
			}
			else // else , just increament the counter
				newCount++;
		}
	_continue:
		freeFileEntry(&local, 1);
	}
	freeFileEntry(mybuf, count);
	free(mybuf);
	return newCount;
}

///////////////////// FUNCTIONS RELATED TO COMMITS/BRANCH/CHECKOUT/... ////////////////////////

void copyGitObjectArray(GitObjectArray *dest, GitObjectArray *src)
{
	if (dest && src)
	{
		dest->arr = malloc(sizeof(GitObject) * (src->len));
		dest->len = src->len;
		for (int i = 0; i < src->len; i++)
		{
			dest->arr[i] = src->arr[i];
			dest->arr[i].file.path = strDup(src->arr[i].file.path);
		}
	}
}

void freeGitObjectArray(GitObjectArray *array)
{
	if (array && array->arr)
	{
		for (int i = 0; i < array->len; i++)
			if (array->arr[i].file.path)
				free(array->arr[i].file.path);
		free(array->arr);
	}
}

Commit *createCommit(GitObjectArray *filesToCommit, constString username, constString email, constString message, uint64_t mergedHash)
{
	if (curRepository->deatachedHead)
		return NULL;

	HEAD *head = &(curRepository->head);
	Commit *newCommit = malloc(sizeof(Commit));
	newCommit->hash = generateUniqueId(6);
	newCommit->prev = head->hash;
	newCommit->branch = strDup(head->branch);
	newCommit->message = strDup(message);
	newCommit->username = strDup(username);
	newCommit->useremail = strDup(email);
	newCommit->time = time(NULL);
	newCommit->mergedCommit = mergedHash;
	copyGitObjectArray(&newCommit->commitedFiles, filesToCommit);

	// Update head files
	copyGitObjectArray(&newCommit->headFiles, &curRepository->head.headFiles);
	for (int i = 0; i < newCommit->commitedFiles.len; i++)
	{
		GitObject *sf = &(newCommit->commitedFiles.arr[i]);
		bool found = false;
		for (int j = 0; j < newCommit->headFiles.len; j++)
		{
			if (!strcmp(newCommit->headFiles.arr[j].file.path, sf->file.path))
			{
				found = true;
				String __p = newCommit->headFiles.arr[j].file.path;
				newCommit->headFiles.arr[j] = *sf;
				newCommit->headFiles.arr[j].file.path = __p;
				break;
			}
		}
		if (!found)
		{
			ADD_EMPTY(newCommit->headFiles.arr, newCommit->headFiles.len, GitObject);
			newCommit->headFiles.arr[newCommit->headFiles.len - 1] = *sf;
			newCommit->headFiles.arr[newCommit->headFiles.len - 1].file.path = strDup(sf->file.path);
		}
	}

	// Create commit file path
	char commitPath[PATH_MAX];
	sprintf(commitPath, "%s/." PROGRAM_NAME "/commits/%06lx", curRepository->absPath, newCommit->hash);
	systemf("mkdir -p \"%s/." PROGRAM_NAME "/commits\"", curRepository->absPath);
	systemf("touch \"%s\"", commitPath);

	// Open and write to the commit file
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
		if (newCommit->mergedCommit)
			fprintf(commitFile, "[perv]:%06lx:[merged]:%06lx\n", newCommit->prev, newCommit->mergedCommit);
		else
			fprintf(commitFile, "[perv]:%06lx\n", newCommit->prev);
		fprintf(commitFile, "[message]:[%s]\n", message);
		fprintf(commitFile, "[%u]:[%u]\n", newCommit->commitedFiles.len, newCommit->headFiles.len);
		for (int i = 0; i < newCommit->commitedFiles.len; i++)
		{
			GitObject *sf = &(newCommit->commitedFiles.arr[i]);
			// <path>:<timeModif>:<perm>:<10digitHash>
			fprintf(commitFile, "%s:%ld:%u:%s\n", sf->file.path, sf->file.dateModif, sf->file.permission, sf->hashStr);
		}
		fputs("\n", commitFile);
		for (int i = 0; i < newCommit->headFiles.len; i++)
		{
			GitObject *sf = &(newCommit->headFiles.arr[i]);
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
		char str[STR_LINE_MAX];
		fgets(str, STR_LINE_MAX, commitFile);
		commit.prev = 0xFFFFFF;
		commit.mergedCommit = 0;
		sscanf(str, "[perv]:%lx:[merged]:%lx\n", &(commit.prev), &(commit.mergedCommit));
		sscanf(str, "[perv]:%lx", &(commit.prev));
		if (!commit.prev)
			throw(0);
		fscanf(commitFile, "[message]:[%[^]]]\n", message);
		if (!*message)
			throw(0);
		if (fscanf(commitFile, "[%u]:[%u]\n", &(commit.commitedFiles.len), &(commit.headFiles.len)) != 2)
			throw(0);

		commit.hash = hash;
		commit.username = strDup(name);
		commit.useremail = strDup(email);
		commit.branch = strDup(branch);
		commit.message = strDup(message);

		commit.commitedFiles.arr = malloc(sizeof(GitObject) * commit.commitedFiles.len);
		commit.headFiles.arr = malloc(sizeof(GitObject) * commit.headFiles.len);

		for (int i = 0; i < commit.commitedFiles.len; i++)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			fscanf(commitFile, "%[^:]:%ld:%u:%s\n", filePath, &timeM, &perm, hash);

			GitObject *sf = &(commit.commitedFiles.arr[i]);
			sf->file.path = strDup(filePath);
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.isDir = (strcmp("dirdirdird", hash) == 0);
			sf->file.permission = perm;
		}

		for (int i = 0; i < commit.headFiles.len; i++)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			fscanf(commitFile, "\n%[^:]:%ld:%u:%s\n", filePath, &timeM, &perm, hash);

			GitObject *sf = &(commit.headFiles.arr[i]);
			sf->file.path = strDup(filePath);
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.isDir = (strcmp("dirdirdird", hash) == 0);
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
		freeGitObjectArray(&object->commitedFiles);
		freeGitObjectArray(&object->headFiles);
		free(object);
	}
}

int listBranches(String *namesDest, uint64_t *hashesDest)
{
	int brCount = -1;
	char path[PATH_MAX];
	// Construct the path for the branches file
	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");
	// Ensure the branches file exists
	systemf("touch \"%s\"", path);
	tryWithFile(branchesFile, path, ({ return -1; }), __retTry)
	{
		// Loop through branches file lines and read names and hashes
		do
			namesDest[++brCount] = malloc(STR_LINE_MAX);
		while (fscanf(branchesFile, "%[^:]:%lx\n", namesDest[brCount], hashesDest + brCount) != EOF);
		throw(brCount);
	}
	return 0;
}

int setBranchHead(constString branchName, uint64_t commitHash)
{
	char path[PATH_MAX];

	// Construct the path for the branches file
	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");

	// Ensure the branches file exists
	systemf("touch \"%s\"", path);
	tryWithFile(branchesFile, path, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char content[STR_LINE_MAX];
		sprintf(content, "%s:*", branchName);

		// Search for the branch in the branches file
		int result = searchLine(branchesFile, content);

		// Format the content with the branch name and commit hash
		sprintf(content, "%s:%06lx\n", branchName, commitHash);

		// If the branch does not exist, append it to the file, otherwise replace the line
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
	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/branches");
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
			strcat_s(pattern, branchName, ":%lx");
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
	// Get the current commit hash of the branch head
	uint64_t curHash = getBranchHead(branchName);

	// Iterate through previous commits based on the order parameter
	for (int i = 0; i < order; i++)
	{
		// Retrieve the commit information for the current hash
		Commit *commit = getCommit(curHash);

		// If the commit is not found, set the hash to 0xFFFFFF and break the loop
		if (commit == NULL)
		{
			curHash = 0xFFFFFF;
			break;
		}

		// Update the current hash to the previous commit's hash
		curHash = commit->prev;

		// free
		freeCommitStruct(commit);
	}
	return curHash;
}

GitObject *getHEADFile(constString path, GitObjectArray head)
{
	for (int i = 0; i < head.len; i++)
		if (!strcmp(head.arr[i].file.path, path))
			return &(head.arr[i]);
	return NULL;
}

int fetchHEAD()
{
	char path[PATH_MAX];
	curRepository->deatachedHead = false;
	curRepository->head.branch = "master";
	curRepository->head.hash = 0xFFFFFF;
	curRepository->head.headFiles.arr = NULL;
	curRepository->head.headFiles.len = 0;

	strcat_s(path, curRepository->absPath, "/." PROGRAM_NAME "/HEAD");
	systemf("touch \"%s\"", path);

	char HEAD_content[STR_LINE_MAX] = {0};
	tryWithFile(HEADfile, path, ({ return ERR_FILE_ERROR; }), __retTry)
		fgets(HEAD_content, STR_LINE_MAX, HEADfile);

	Commit *head = NULL;
	char branch[STR_LINE_MAX];
	if (isMatch(HEAD_content, "branch/*"))
	{
		sscanf(HEAD_content, "branch/%s", branch);
		curRepository->head.branch = strDup(branch);
		curRepository->head.hash = getBranchHead(branch) & 0xFFFFFF;
		head = getCommit(curRepository->head.hash);
	}
	else if (isMatch(HEAD_content, "commit/*:*"))
	{
		curRepository->deatachedHead = true;
		sscanf(HEAD_content, "commit/%lx:%s", &curRepository->head.hash, branch);
		head = getCommit(curRepository->head.hash);
		if (head)
			curRepository->head.branch = strDup(branch);
	}
	else
		systemf("echo branch/master>\"%s/." PROGRAM_NAME "/HEAD\"", curRepository->absPath);

	if (head)
	{
		// obtain head files
		curRepository->head.headFiles.arr = malloc(sizeof(GitObject) * head->headFiles.len);
		curRepository->head.headFiles.len = head->headFiles.len;
		for (int i = 0; i < head->headFiles.len; i++)
		{
			curRepository->head.headFiles.arr[i] = head->headFiles.arr[i];
			curRepository->head.headFiles.arr[i].file.path = strDup(head->headFiles.arr[i].file.path);
		}
		freeCommitStruct(head);
	}
	return ERR_NOERR;
}

ChangeStatus getChangesFromHEAD(constString path, GitObjectArray head)
{
	GitObject *headFile = getHEADFile(path, head);

	char abspath[PATH_MAX];
	strcat_s(abspath, curRepository->absPath, "/", path);
	if (access(abspath, F_OK) != 0) // real file Not Exist
	{
		if (headFile == NULL) // This state should not occur (except calling with the wrong path)
			return NOT_CHANGED;
		if (!(headFile->file.isDeleted))
			return DELETED; // The real file was committed before but is deleted from the working directory
		else
			return NOT_CHANGED; // Deleted and Commited before!
	}

	if (headFile == NULL)
		return ADDED; // The file was added after the HEAD commit

	else
	{
		// Committed file and real file are present, compare them
		// absolute
		char headObjAbsPath[PATH_MAX];
		FileEntry realFile = getFileEntry(abspath, NULL);
		freeFileEntry(&realFile, 1);
		strcat_s(headObjAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", headFile->hashStr);

		if (headFile->file.isDeleted)
			return ADDED;
		else if (!isFilesSame(abspath, headObjAbsPath))
			return MODIFIED;
		else if (realFile.permission != headFile->file.permission)
			return PERM_CHANGED;
		else
			return NOT_CHANGED;
	}
}

bool isWorkingTreeModified()
{
	char manifestFilePath[PATH_MAX];
	strcat_s(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
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
			if (getChangesFromHEAD(buf, curRepository->head.headFiles))
			{
				flag = true;
				break;
			}
		}
		throw(flag);
	}
	return false;
}

int applyToWorkingDir(GitObjectArray head)
{
	char manifestFilePath[PATH_MAX];
	strcat_s(manifestFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
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
			strcat_s(absPath, curRepository->absPath, "/", buf);
			ChangeStatus state = getChangesFromHEAD(buf, head);
			GitObject *sf = getHEADFile(buf, head);
			switch (state)
			{
			case ADDED:			 // We have to remove this file from working tree
				remove(absPath); // DELETE THE FILE IN WORKING TREE !!
				break;
			case DELETED:  // We have to add this file to working tree
			case MODIFIED: // We have to update this file at working tree
				char objAbsPath[PATH_MAX];
				strcat_s(objAbsPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", sf->hashStr);
				copyFile(objAbsPath, absPath, NULL); // REPLACE THE FILE IN WORKING TREE WITH HEAD ONE !!
				struct utimbuf newTime;
				newTime.actime = time(NULL);		  // Access time set to now
				newTime.modtime = sf->file.dateModif; // Modification time is set to the original timestamp
				utime(absPath, &newTime);
			case PERM_CHANGED:
				chmod(absPath, sf->file.permission);
				break;
			}
		}
	}
	return ERR_NOERR;
}

///////////////////// FUNCTIONS RELATED TO TAG/DIFF/MERGE ////////////////////////

String getMergeDestination(constString branch)
{
	uint64_t hash = getBranchHead(branch);
	Commit *c = getCommit(hash);
	if (c == NULL)
		return false;
	Commit *mergedCommit = getCommit(c->mergedCommit);
	bool isMerged = mergedCommit && (strcmp(mergedCommit->branch, branch) == 0);
	freeCommitStruct(mergedCommit);
	String res = isMerged ? strDup(c->branch) : NULL;
	freeCommitStruct(c);
	return res;
}

void printDiff(Diff *diff, constString f1PathToShow, constString f2PathToShow)
{
	printf("File " _CYAN _BOLD "%s" _UNBOLD _DFCOLOR " vs File " _YEL _BOLD "%s" _UNBOLD _DFCOLOR " :\n", f1PathToShow, f2PathToShow);

	if (diff->addedCount != 0 || diff->removedCount != 0)
	{
		printf("<<<<<<<<<\n");
		int i = 0;
		for (i = 0; i < diff->removedCount && i < diff->addedCount; i++)
		{
			printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", f1PathToShow, diff->lineNumberRemoved[i]);
			printf(_DIM "< " _RST _CYAN "%s\n" _RST, diff->linesRemoved[i]);
			printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", f2PathToShow, diff->lineNumberAdded[i]);
			printf(_DIM "> " _RST _YEL "%s\n" _RST, diff->linesAdded[i]);
		}
		for (; i < diff->removedCount; i++)
		{
			printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", f1PathToShow, diff->lineNumberRemoved[i]);
			printf(_DIM "< " _RST _CYAN "%s\n" _RST, diff->linesRemoved[i]);
		}
		for (; i < diff->addedCount; i++)
		{
			printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", f2PathToShow, diff->lineNumberAdded[i]);
			printf(_DIM "> " _RST _YEL "%s\n" _RST, diff->linesAdded[i]);
		}
		printf(">>>>>>>>>\n");
	}
	else
		printf(_GRN "No Difference found! (in Text Mode and ignoring empty lines)\n" _RST);

	return;
}

ConflictingStatus getConflictingStatus(GitObject *targetObj, GitObjectArray base, Diff *diffDest)
{
	GitObject *baseObj = getHEADFile(targetObj->file.path, base);

	if (baseObj == NULL) // New object
		return NEW_FILE;
	else if (!strcmp(targetObj->hashStr, baseObj->hashStr)) // same object!  no conflicts here :D
		return SAME_BINARY;
	else if (!strcmp(targetObj->hashStr, "dddddddddd")) // the target file is marked as deleted
		return REMOVED_IN_TARGET;
	else if (!strcmp(baseObj->hashStr, "dddddddddd")) // the file had been deleted from base
		return REMOVED_IN_BASE;
	else // the file is persent on  both branches but with different object ref
	{
		char baseObjPath[PATH_MAX], targetObjPath[PATH_MAX];
		strcat_s(baseObjPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", baseObj->hashStr);
		strcat_s(targetObjPath, curRepository->absPath, "/." PROGRAM_NAME "/objects/", targetObj->hashStr);
		if (isFilesSame(baseObjPath, targetObjPath))
			return SAME_BINARY;
		else
		{
			Diff diff = getDiff(baseObjPath, targetObjPath, 1, -1, 1, -1);
			if (diff.addedCount == 0 && diff.linesRemoved == 0)
			{
				freeDiffStruct(&diff);
				return SAME_TEXT;
			}
			else
			{
				if(diffDest) *diffDest = diff;
				return CONFLICT;
			}
		}
	}
}

// Comparator function for qsort Tags (Name Ascending)
int __tag_sort_comparator(const void *a, const void *b)
{
	return strcasecmp((((Tag *)a)->tagname), ((Tag *)b)->tagname);
}

int listTags(Tag **destBuf, uint64_t commitHash)
{
	Tag *result = NULL;
	uint count = 0;
	char tagManifestPath[PATH_MAX];
	strcat_s(tagManifestPath, curRepository->absPath, "/.neogit/tags");
	systemf("touch \"%s\"", tagManifestPath);
	tryWithFile(tagManifest, tagManifestPath, ({ return 0; }), ({ return count; }))
	{
		char buf[STR_LINE_MAX];
		while (fgets(buf, STR_LINE_MAX, tagManifest))
		{
			strtrim(buf);
			char tag_name[100], message[STR_LINE_MAX], authorName[STR_LINE_MAX], authorEmail[STR_LINE_MAX];
			uint64_t hash = 0;
			time_t time = 0;

			if (sscanf(buf, "[%[^]]]:[%[^]]]:%lx:%[^:]:%[^:]:%ld", tag_name, message, &hash, authorName, authorEmail, &time) != 6)
				continue;

			if (commitHash && (hash != commitHash)) // if commithash filter provided, check the hash
				continue;

			ADD_EMPTY(result, count, Tag);
			result[count - 1].tagname = strDup(tag_name);
			result[count - 1].message = strDup(message);
			result[count - 1].authorName = strDup(authorName);
			result[count - 1].authorEmail = strDup(authorEmail);
			result[count - 1].tagTime = time;
			result[count - 1].commitHash = hash;
		}
		qsort(result, count, sizeof(Tag), __tag_sort_comparator); // sort by name ascending
		if (destBuf)
			*destBuf = result;
		else
			freeTagStruct(result, count);
		throw(0);
	}
	return count;
}

Tag *getTag(constString tag_name)
{
	Tag *result = NULL;
	char tagManifestPath[PATH_MAX];
	strcat_s(tagManifestPath, curRepository->absPath, "/.neogit/tags");
	systemf("touch \"%s\"", tagManifestPath);
	tryWithFile(tagManifest, tagManifestPath, ({ return NULL; }), ({ return result; }))
	{
		char pattern[STR_LINE_MAX], format[STR_LINE_MAX];
		sprintf(pattern, "[%s]:[*]:*:*:*:*", tag_name);
		strcat_s(format, "[", tag_name, "]:[%[^]]]:%lx:%[^:]:%[^:]:%ld");
		int res = searchLine(tagManifest, pattern);
		if (res == -1) // if not found
			throw(0);
		else // if found
		{
			SEEK_TO_LINE(tagManifest, res);
			char buf[STR_LINE_MAX];
			if (!fgets(buf, STR_LINE_MAX, tagManifest))
				throw(0); // return NULL
			char message[STR_LINE_MAX], authorName[STR_LINE_MAX], authorEmail[STR_LINE_MAX];
			uint64_t hash = 0;
			time_t time = 0;
			if (sscanf(buf, format, message, &hash, authorName, authorEmail, &time) != 5)
				throw(0); // return NULL

			result = malloc(sizeof(Tag));
			result->tagname = strDup(tag_name);
			result->message = strDup(message);
			result->commitHash = hash;
			result->tagTime = time;
			result->authorName = strDup(authorName);
			result->authorEmail = strDup(authorEmail);
		}

		throw(0);
	}
	return result;
}

int setTag(constString tag_name, constString message, uint64_t commitHash, constString author_name, constString author_email, time_t time)
{
	char tagManifestPath[PATH_MAX];
	strcat_s(tagManifestPath, curRepository->absPath, "/.neogit/tags");
	systemf("touch \"%s\"", tagManifestPath);
	tryWithFile(tagManifest, tagManifestPath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		char format[STR_LINE_MAX], lineStr[STR_LINE_MAX];
		sprintf(format, "[%s]:[*]:*:*:*:*", tag_name);
		sprintf(lineStr, "[%s]:[%s]:%06lx:%s:%s:%ld\n", tag_name, message, commitHash, author_name, author_email, time);
		int res = searchLine(tagManifest, format);
		if (res == -1)													  // if not found
			fputs(lineStr, freopen(tagManifestPath, "ab+", tagManifest)); // append line to the file
		else															  // if found
			replaceLine(tagManifest, res, lineStr);						  // replace line

		throw(ferror(tagManifest));
	}
	return ERR_NOERR;
}

void freeTagStruct(Tag *array, uint length)
{
	if(!array) return;
	for (int i = 0; i < length; i++)
	{
		if (array[i].tagname)
			free(array[i].tagname);
		if (array[i].message)
			free(array[i].message);
		if (array[i].authorName)
			free(array[i].authorName);
		if (array[i].authorEmail)
			free(array[i].authorEmail);
	}
	free(array);
}
