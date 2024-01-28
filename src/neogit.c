#include "neogit.h"

// Global variable for cwd (Used in other c files - valued in begining of main())
String curWorkingDir = NULL;
// Global variable for current repository (Used in other c files - valued in begining of main())
String curRepoPath = NULL;

String obtainRepository(constString workDir)
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

			// If we reach root, or error in navigating to parent, break loop;
			if (isMatch(getcwd(NULL, PATH_MAX), "/") || chdir("..") != 0)
				break;
		}

		// Change back to the original working directory (Pop from temp variable)
		chdir(tmpWorkDir);
	}
	return repoPath;
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
		configPath = strConcat(curRepoPath, "/." PROGRAM_NAME "/config");
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
	tryWithString(configPath, ـgetconfigpath(global), ({ return ERR_FILE_ERROR; }), ({ return _ERR; }))
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

	if (!curRepoPath) // Repo not initialized
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
	tryWithString(configPath, ـgetconfigpath(global), ({ return ERR_FILE_ERROR; }), ({ return _ERR; }))
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
	if (curRepoPath)
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