/*******************************
 *         phase1.c            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "phase1.h"

extern String curWorkingDir;	  // Declared in neogit.c
extern Repository *curRepository; // Declared in neogit.c

// A callback function for processTree -> print staging status of working tree files
void __stage_print_func(FileEntry *element)
{
	if (isMatch(element->path, "." PROGRAM_NAME)) // .neogit Directory
		printf(_MAGNTA _BOLD "." PROGRAM_NAME _UNBOLD " (NeoGIT Directory)\n" _RST);
	else if (element->isDir) // Directory
		printf(_BLU _BOLD "%s" _UNBOLD " (dir)\n" _RST, getFileName(element->path));
	else if (!isTrackedFile(element->path)) // Untracked
		printf(_GRN _BOLD "%s" _UNBOLD " → Untraked\n" _RST, getFileName(element->path));
	else if (getChangesFromStaging(element->path)) // Unstaged (Modified)
		printf(_YEL _BOLD "%s" _UNBOLD " → Modified\n" _RST, getFileName(element->path));
	else if (getChangesFromHEAD(element->path, curRepository->head.headFiles)) // Staged
		printf(_BOLD "%s" _UNBOLD " → Staged\n" _RST, getFileName(element->path));
	else // not changed from head commit
		printf(_BOLD _DIM "%s" _UNBOLD _DIM " → Commited\n" _RST, getFileName(element->path));
}

// A callback function for processTree -> print status of changed files
void __status_print_func(FileEntry *element)
{
	String name = getFileName(element->path);

	if (isMatch(element->path, "." PROGRAM_NAME)) // .neogit Directory
		printf(_MAGNTA _BOLD "." PROGRAM_NAME _UNBOLD " (NeoGIT Directory)\n" _RST);
	else if (isMatch(element->path, "."))
		printf(_BLU "(Repository Root)\n" _RST);
	else if (element->isDir) // Directory
		printf(_BLU "%s  (dir)\n" _RST, name);
	else
	{
		ChangeStatus changes = getChangesFromStaging(element->path);
		bool staged = !changes; // FASLE if there is change between working dir and staging area
		if (staged)
			changes = getChangesFromHEAD(element->path, curRepository->head.headFiles);

		switch (changes)
		{
		case ADDED:
			printf(_GRN _BOLD "%s" _UNBOLD " → A%c\n" _RST, name, staged ? '+' : '-');
			break;
		case MODIFIED:
			printf(_YEL _BOLD "%s" _UNBOLD " → M%c\n" _RST, name, staged ? '+' : '-');
			break;
		case DELETED:
			printf(_RED _BOLD "%s" _UNBOLD " → D%c\n" _RST, name, staged ? '+' : '-');
			break;
		case PERM_CHANGED:
			printf(_CYAN _BOLD "%s" _UNBOLD " → T%c\n" _RST, name, staged ? '+' : '-');
			break;
		}
	}
}

// A comparator function for qsort -> sort commits by date deascending
int __cmpCommitsByDateDescending(const void *a, const void *b)
{
	Commit *first = *((Commit **)a), *second = *((Commit **)b);
	return second->time - first->time;
}

// Parse log command options and put them in struct LogOption *dest
int __parseLogOptions(int argc, constString argv[], LogOptions *dest)
{
	LogOptions options = {-1, "*", "*", 0, time(NULL), "*"}; // default options

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-n") == 0 && i + 1 < argc)
			options.n = atoi(argv[++i]);
		else if (strcmp(argv[i], "-branch") == 0 && i + 1 < argc)
			options.branch = argv[++i];
		else if (strcmp(argv[i], "-author") == 0 && i + 1 < argc)
			options.author = argv[++i];
		else if (strcmp(argv[i], "-since") == 0 && i + 1 < argc)
		{
			time_t t = parseDateTimeAuto(argv[++i]);
			if (t == ERR_ARGS_MISSING)
				return t;
			options.since = t;
		}
		else if (strcmp(argv[i], "-before") == 0 && i + 1 < argc)
		{
			time_t t = parseDateTimeAuto(argv[++i]);
			if (t == ERR_ARGS_MISSING)
				return t;
			options.before = t;
		}
		else if (strcmp(argv[i], "-search") == 0 && i + 1 < argc)
			options.search = argv[++i];
		else
			return ERR_ARGS_MISSING;
	}
	*dest = options;
	return ERR_NOERR;
}

int command_init(int argc, constString argv[], bool performActions)
{
	if (performActions)
	{
		// Repo already exist
		if (curRepository)
		{
			printError("The repository is already initialized in " _BOLD "\"%s\"" _UNBOLD ".", curRepository->absPath);
			return ERR_ALREADY_EXIST;
		}

		// make .neogit folder and create repo
		char neogitFolder[PATH_MAX], tmpPath[PATH_MAX];
		mkdir(strcat_s(neogitFolder, curWorkingDir, "/." PROGRAM_NAME), 0775);
		mkdir(strcat_s(tmpPath, neogitFolder, "/stage"), 0775);
		mkdir(strcat_s(tmpPath, neogitFolder, "/objects"), 0775);
		mkdir(strcat_s(tmpPath, neogitFolder, "/commits"), 0775);
		systemf("touch ." PROGRAM_NAME "/config");
		systemf("touch ." PROGRAM_NAME "/tracked");
		systemf("touch ." PROGRAM_NAME "/HEAD");
		systemf("touch ." PROGRAM_NAME "/branches");
		systemf("touch ." PROGRAM_NAME "/tags");

		int err = 0;
		err |= obtainRepository(curWorkingDir);
		err |= system("neogit branch master");
		err |= system("neogit checkout master >/dev/null");
		printf("\n");
		return err;
	}
	return ERR_NOERR;
}

int command_config(int argc, constString argv[], bool performActions)
{
	// Check if the command is related to alias or user configurations
	bool isAlias = false;
	uint keyArgIndex = checkAnyArgument("user.*");
	if (!keyArgIndex)
	{
		if (keyArgIndex = checkAnyArgument("alias.*"))
			isAlias = true;
	}

	// Check for global flag and repository flag
	uint globalFlagIndex = checkAnyArgument("--global");
	if (!keyArgIndex || (!checkAnyArgument("-R") && globalFlagIndex == keyArgIndex + 1))
		return ERR_ARGS_MISSING;

	// Check if the repository is initialized for local configurations
	if (performActions && !globalFlagIndex && !curRepository)
		return ERR_NOREPO;

	tryWithString(key, malloc(strlen(argv[keyArgIndex]) + 1), ({ return ERR_MALLOC; }), __retTry)
	{
		// Extract key and validate it
		uint invalid = strValidate(key, argv[keyArgIndex], VALID_CHARS);
		if (isEmpty(key))
			throw(ERR_ARGS_MISSING);

		if (checkAnyArgument("-R")) // remove config
		{
			if (!performActions)
				throw(ERR_NOERR);
			int res = removeConfig(key, globalFlagIndex);
			if (res == ERR_NOERR)
				printf("The config [" _YEL "%s" _DFCOLOR "] successfully removed from %s!\n", key, globalFlagIndex ? "global" : "local");
			else if (res == ERR_CONFIG_NOTFOUND)
				printError("The config [%s] not found in %s!", key, globalFlagIndex ? "global" : "local");
			else
				printError("ERROR! In remove config [%s] from %s", key, globalFlagIndex ? "global" : "local");
			throw(res);
		}

		// Extract value and validate it
		tryWithString(value, malloc(strlen(argv[keyArgIndex + 1]) + 1), throw(ERR_MALLOC), throw(_err))
		{
			invalid += strValidate(value, argv[keyArgIndex + 1], "a-zA-Z0-9.@!~( )/_-");
			if (invalid)
				if (performActions)
					printWarning("You're Entered at least one invalid character in value. It will be removed.");
			if (isEmpty(value))
				throw(ERR_ARGS_MISSING);

			// Check and process alias commands
			if (isAlias)
			{
				String alias_argv[20];
				tryWithString(commandDuplicate, strDup(value), {}, throw(_ERR))
				{
					uint alias_argc = tokenizeString(commandDuplicate, " ", alias_argv);
					// Check the command syntax
					int result = process_command(alias_argc, (constString *)alias_argv, false);
					if (result != ERR_NOERR)
						printError("Error in set alias: Command [%s] is incorrect!", value);

					throw(result);
				}
			}

			//  if we don't have to perform actions, return OK (syntax OK)
			if (!performActions)
				throw(ERR_NOERR);

			// Set the configuration value
			time_t now = time(NULL);
			uint errcode = setConfig(key, value, now, globalFlagIndex);
			if (errcode == ERR_NOERR)
				printf("The %s config [" _CYAN "%s" _DFCOLOR "] successfully set to " _CYAN "%s" _DFCOLOR " at time %ld.\n" _RST,
					   globalFlagIndex ? "global" : "local", key, value, now);
			else
				printError("ERROR! In set config [%s] to [%s]", key, value);

			throw(errcode);
		}
	}
	return ERR_NOERR;
}

int command_add(int argc, constString argv[], bool performActions)
{
	if (checkArgument(1, "-n")) // List file and show staging state
	{
		// Check syntax
		int curDepth;
		if (argc != 3 && argc != 4)
			return ERR_ARGS_MISSING;
		else if ((curDepth = atoi(argv[2])) <= 0)
			return ERR_ARGS_MISSING;
		else if (!performActions)
			return ERR_NOERR;

		printf("\n");
		FileEntry root = getFileEntry(".", NULL);
		processTree(&root, atoi(argv[2]), ls, true, __stage_print_func);
		printf("\n");
		freeFileEntry(&root, 1);
		return ERR_NOERR;
	}

	// Re Stage all modified tracked files
	if (checkArgument(1, "-redo"))
	{
		// Check syntax
		if (argc != 2)
			return ERR_ARGS_MISSING;
		else if (!performActions)
			return ERR_NOERR;

		char tracklistFilePath[PATH_MAX];
		strcat_s(tracklistFilePath, curRepository->absPath, "/." PROGRAM_NAME "/tracked");
		systemf("touch \"%s\"", tracklistFilePath);
		tryWithFile(manifestFile, tracklistFilePath, ({ return ERR_FILE_ERROR; }), __retTry)
		{
			char buf[PATH_MAX];
			while (fgets(buf, sizeof(buf), manifestFile) != NULL)
				if (getChangesFromStaging(strtrim(buf)))
				{
					int res = addToStage(buf);
					if (res == ERR_NOERR)
						printf("Modified file restaged: " _CYAN "%s \n"_RST, buf);
					else
						printError("Error! in adding file: " _BOLD "%s" _UNBOLD ".\n", buf);
				}
		}

		return ERR_NOERR;
	}

	// Add files
	bool firstCall = true;
	if (checkArgument(1, "-f") || checkArgument(1, "-ff"))
	{
		if (checkArgument(1, "-ff"))
			firstCall = false;
		argc -= 2;
		argv += 2;
	}
	else
	{
		argc--;
		argv++;
	}

	// Check syntax
	if (argc <= 0)
		return ERR_ARGS_MISSING;
	else if (!performActions)
		return ERR_NOERR;
	else if (!curRepository)
		return ERR_NOREPO;

	// backup from old staged area (for undo actions)
	if (firstCall)
	{
		_LIST_FILES_CHANGED_FROM_STAGE;
		if (lsChangedFiles(NULL, curRepository->absPath))
			backupStagingArea();
		_LIST_FILES_CHANGED_FROM_HEAD;
	}

	while (argc--)
	{
		constString argument = *(argv++);
		// dest will be absolute path or relative to cwd
		char dest[PATH_MAX];
		uint illegal = strValidate(dest, argument, VALID_CHARS);
		strtrim(dest);

		if (illegal || !*dest)
		{
			printWarning("File or directory " _BOLD "\"%s\"" _UNBOLD " has illegal characters! It has been ignored.", argument);
			continue;
		}

		String relPath = normalizePath(dest, curRepository->absPath);

		// entry paths will be absolute
		int result = -1;
		FileEntry *entries = NULL;
		if (relPath)
		{
			result = lsWithHead(&entries, dest);
			free(relPath);
		}
		if (result == -1) // error
			printError("Error! " _BOLD "\"%s\"" _UNBOLD " : No such file or directory!", argument);
		else if (result == -2) // add file
		{
			// relative to the repo
			FileEntry fileE = getFileEntry(dest, curRepository->absPath);

			if (fileE.path == NULL)
				printError("Error in adding File " _BOLD "\"%s\"" _UNBOLD, argument);
			else
			{
				// .gitignore patterns are handled here.
				if (isGitIgnore(&fileE))
					continue;
				int res = 0;
				if (getChangesFromStaging(fileE.path))
				{
					trackFile(fileE.path);
					res = addToStage(fileE.path);
					if (res == ERR_NOERR)
						printf("File Added to stage: " _CYAN "%s " _RST "\n", fileE.path);
					else
						printError("Error! in adding file: " _BOLD "%s" _UNBOLD ".\n", fileE.path);
				}
				else
				{
					printf(_DIM "No changes : %s\n" _UNBOLD _RST, fileE.path);
				}
				freeFileEntry(&fileE, 1);
			}
		}
		else // add folders recursive
		{
			String new_argv[result + 2];
			new_argv[0] = "add";
			new_argv[1] = "-ff";
			int new_argc = 2;
			for (int i = 0; i < result; i++)
			{
				// Don't Add .neogit folder!
				if (entries[i].isDir && strcmp(getFileName(entries[i].path), "." PROGRAM_NAME) == 0)
					continue;

				// .gitignore patterns are handled here.
				if (isGitIgnore(&entries[i]))
					continue;

				new_argv[new_argc++] = entries[i].path;
			}

			// Add childs recursive
			command_add(new_argc, (constString *)new_argv, true);
			freeFileEntry(entries, result);
			if (result)
				free(entries);
		}
	}
	if (firstCall)
		printf("\n\n");
	return ERR_NOERR;
}

int command_reset(int argc, constString argv[], bool performActions)
{
	// undo command (restore last backup and shift backups down)
	if (checkArgument(1, "-undo"))
	{
		// Check syntax
		if (argc != 2)
			return ERR_ARGS_MISSING;
		else if (!performActions)
			return ERR_NOERR;

		int res = restoreStageingBackup();
		if (res == ERR_NOERR)
			printf("The last staged files, have been unstaged successfully.\n");
		else if (res == ERR_NOT_EXIST)
			printWarning("No recent added file to undo.");
		else
			printError("Error : in restoring stage backup.");
		return res;
	}

	if (checkArgument(1, "-f") || checkArgument(1, "-ff"))
	{
		argc -= 2;
		argv += 2;
	}
	else
	{
		argc--;
		argv++;
	}

	// Check syntax
	if (argc <= 0)
		return ERR_ARGS_MISSING;
	else if (!performActions)
		return ERR_NOERR;
	else if (!curRepository)
		return ERR_NOREPO;

	while (argc--)
	{
		constString argument = *(argv++);
		// dest will be absolute or relative to cwd
		char dest[PATH_MAX];
		uint illegal = strValidate(dest, argument, VALID_CHARS);
		strtrim(dest);

		if (illegal || !*dest)
		{
			printWarning("File or directory " _BOLD "\"%s\"" _UNBOLD " has illegal characters! It has been ignored.", argument);
			continue;
		}

		String relPath = normalizePath(dest, curRepository->absPath);

		// entry paths will be absolute
		int result = -1;
		FileEntry *entries = NULL;
		if (relPath)
		{
			result = lsWithHead(&entries, dest);
			free(relPath);
		}

		if (result == -1) // error
			printError("Error! " _BOLD "\"%s\"" _UNBOLD " : No such file or directory!", argument);
		else if (result == -2) // reset file
		{
			// relative to the repo
			FileEntry fileE = getFileEntry(dest, curRepository->absPath);

			if (fileE.path == NULL)
				printError("Error in adding File " _BOLD "\"%s\"" _UNBOLD, argument);
			else
			{
				// .gitignore patterns are handled here.
				if (isGitIgnore(&fileE))
					continue;

				int res = 0;
				res = removeFromStage(fileE.path);
				if (res == ERR_NOERR)
					printf("File removed from stage: " _CYAN "%s\n" _RST, fileE.path);
				else if (res == ERR_NOT_EXIST)
					printf(_DIM "Not staged: %s" _UNBOLD _RST "\n", fileE.path);
				else
					printError("Error! in removing file: " _BOLD "%s" _UNBOLD ".\n", fileE.path);
				freeFileEntry(&fileE, 1);
			}
		}
		else // reset folders recursive
		{
			String new_argv[result + 2];
			new_argv[0] = "reset";
			new_argv[1] = "-ff";
			int new_argc = 2;
			for (int i = 0; i < result; i++)
			{
				// Don't Process .neogit folder!
				if (entries[i].isDir && strcmp(getFileName(entries[i].path), "." PROGRAM_NAME) == 0)
					continue;

				// .gitignore patterns are handled here.
				if (isGitIgnore(&entries[i]))
					continue;

				new_argv[new_argc++] = entries[i].path;
			}

			// remove childs recursive
			command_reset(new_argc, (constString *)new_argv, true);
			freeFileEntry(entries, result);
			if (result)
				free(entries);
		}
	}
	return ERR_NOERR;
}

int command_status(int argc, constString argv[], bool performActions)
{
	if (!performActions)
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	if (!curRepository->deatachedHead)
		printf("On branch " _YELB "'%s'\n\n" _RST, curRepository->head.branch);
	else
		printf(_YELB "YOU ARE IN DEATACHED HEAD MODE!! DONT CHANGE FILES!!!\n\n" _RST);

	if (lsChangedFiles(NULL, curRepository->absPath))
	{
		FileEntry root = getFileEntry(curRepository->absPath, NULL);
		processTree(&root, 15, lsChangedFiles, true, __status_print_func);
		freeFileEntry(&root, 1);
		printf("\n");
	}
	else
		printf(_CYAN "Working tree is clean! No modifications.\n\n" _RST);

	return ERR_NOERR;
}

int command_commit(int argc, constString argv[], bool performActions)
{
	char message[COMMIT_MSG_LEN_MAX + 1];

	// Check message
	if (checkArgument(1, "-m"))
	{
		if (!performActions)
			return ERR_NOERR;
		if (strlen(argv[2]) > COMMIT_MSG_LEN_MAX)
		{
			if (performActions)
				printError("The message longer than COMMIT_MSG_LEN_MAX.");
			return ERR_GENERAL;
		}
		strValidate(message, argv[2], VALID_CHARS);
		if (isEmpty(message))
		{
			printError("The commit message is empty!");
			return ERR_ARGS_MISSING;
		}
	}

	// obtain shortcut message
	else if (checkArgument(1, "-s"))
	{
		if (!performActions)
			return ERR_NOERR;
		char configKey[strlen(argv[2]) + 10];
		tryWithString(shortcutKey, malloc(sizeof(configKey)), ({ return ERR_MALLOC; }), __retTry)
		{
			strValidate(shortcutKey, argv[2], VALID_CHARS);
			if (isEmpty(shortcutKey))
				throw(ERR_ARGS_MISSING);
			strcat_s(configKey, "shortcut/", strtrim(shortcutKey));
		}
		String value = getConfig(configKey);
		if (value)
			strcpy(message, value);
		else
		{
			if (performActions)
				printError("The shortcut key not found!");
			return ERR_GENERAL;
		}
	}
	else
		return ERR_ARGS_MISSING;

	if (!curRepository)
		return ERR_NOREPO;

	// Check the user.name and user.email
	String name = getConfig("user.name");
	String email = getConfig("user.email");
	if (!name || !email)
	{
		printError("You haven't been configured the user information!");
		printError("Please submit your information and configs for NeoGIT!\n");
		printWarning("You must first set them by Command(s) below:");
		if (!name)
			printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.name <yourname>\"" _RST);
		else
			free(name);
		if (!email)
			printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.email <youremail@example.com>\"" _RST);
		else
			free(email);
		return ERR_CONFIG_NOTFOUND;
	}

	if (curRepository->deatachedHead)
	{
		printWarning("Your HEAD is in DEATACHED MODE! You can not commit anything...");
		return ERR_DEATACHED_HEAD;
	}

	if (curRepository->stagingArea.len == 0)
	{
		printWarning("No file staged! Nothing to commit ...");
		free(name);
		free(email);
		return ERR_NOT_EXIST;
	}

	// Perform the commit
	// copy objects from .neogit/stage to .neogit/objects
	for (int i = 0; i < curRepository->stagingArea.len; i++)
	{
		char p1[PATH_MAX], p2[PATH_MAX];
		if (!curRepository->stagingArea.arr[i].file.isDeleted) //  if the file was deleted we don't need it in our object store
			copyFile(strcat_s(p1, "." PROGRAM_NAME "/stage/", curRepository->stagingArea.arr[i].hashStr),
					 strcat_s(p2, "." PROGRAM_NAME "/objects/", curRepository->stagingArea.arr[i].hashStr), curRepository->absPath);
	}

	// submit the commit
	Commit *res = createCommit(&curRepository->stagingArea, name, email, message, 0);
	free(name);
	free(email);

	// if successful , print results
	if (res)
	{
		systemf("rm -r \"%s/." PROGRAM_NAME "/stage\"/*", curRepository->absPath); // remove staged files
		printf("Successfully performed the commit: " _CYANB "'%s'\n" _RST, message);
		char datetime[DATETIME_STR_MAX];
		strftime(datetime, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&res->time));
		printf("Date and Time : " _BOLD "%s\n" _RST, datetime);
		printf("on branch " _CYANB "'%s'" _RST " - commit hash " _CYANB "'%06lx'\n" _RST, curRepository->head.branch, curRepository->head.hash);
		printf(_DIM "[" _BOLD "%u" _UNBOLD _DIM " file(s) commited]\n" _UNBOLD _RST "\n", res->commitedFiles.len);

		freeCommitStruct(res);
	}
	else
	{
		printError("Error in perform commit!");
		return ERR_UNKNOWN;
	}
	return ERR_NOERR;
}

int command_shortcutmsg(int argc, constString argv[], bool performActions)
{
	// Check Syntax
	int shortcut = checkAnyArgument("-s") + 1;
	int message = checkAnyArgument("-m") + 1;
	if (argc != 5 || shortcut == 1 || message == 1)
		return ERR_ARGS_MISSING;

	char configKey[strlen(argv[shortcut]) + 10];
	tryWithString(shortcutKey, malloc(sizeof(configKey)), ({ return ERR_MALLOC; }), __retTry)
	{
		strValidate(shortcutKey, argv[shortcut], VALID_CHARS);
		if (isEmpty(shortcutKey))
			throw(ERR_ARGS_MISSING);
		strcat_s(configKey, "shortcut/", strtrim(shortcutKey));
	}

	char value[strlen(argv[message]) + 1];
	strValidate(value, argv[message], VALID_CHARS);

	if (strlen(value) > COMMIT_MSG_LEN_MAX)
	{
		if (performActions)
			printError("The message longer than COMMIT_MSG_LEN_MAX.\n");
		return ERR_GENERAL;
	}

	if (!performActions)
		return ERR_NOERR;

	if (!curRepository)
		return ERR_NOREPO;

	if (checkArgument(0, "set") && getConfig(configKey))
	{
		printError("The shortcut key already exist!\n");
		return ERR_ALREADY_EXIST;
	}
	else if (checkArgument(0, "replace") && !getConfig(configKey))
	{
		printError("The shortcut key does not exist!\n");
		return ERR_NOT_EXIST;
	}

	// Perform Acions
	int err = setConfig(configKey, value, time(NULL), false);
	if (err == ERR_NOERR)
		printf("The shortcut " _CYAN "%s" _DFCOLOR " successfully set to message " _CYAN "\"%s\"\n\n" _RST, configKey + strlen("shortcut/"), value);
	else
		printError("Error in set shortcut message!\n");

	return err;
}

int command_remove(int argc, constString argv[], bool performActions)
{
	// Check Syntax
	if (argc != 3 || !checkArgument(1, "-s"))
		return ERR_ARGS_MISSING;

	char configKey[strlen(argv[2]) + 10];
	tryWithString(shortcutKey, malloc(sizeof(configKey)), ({ return ERR_MALLOC; }), __retTry)
	{
		strValidate(shortcutKey, argv[2], VALID_CHARS);
		if (isEmpty(shortcutKey))
			throw(ERR_ARGS_MISSING);
		strcat_s(configKey, "shortcut/", strtrim(shortcutKey));
	}

	if (!performActions)
		return ERR_NOERR;

	if (!curRepository)
		return ERR_NOREPO;

	int err = removeConfig(configKey, false);
	if (err == ERR_NOERR)
		printf("The shortcut key " _CYAN "%s" _DFCOLOR " removed successfully!\n\n" _RST, configKey + strlen("shortcut/"));
	else
		printError("Shortcut key does not exist!\n");
	return err;
}

int command_log(int argc, constString argv[], bool performActions)
{
	// Check Syntax
	LogOptions options;
	if (__parseLogOptions(argc, argv, &options) != ERR_NOERR)
		return ERR_ARGS_MISSING;
	if (!performActions)
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	// obtain array of commits
	char commitsPath[PATH_MAX];
	strcat_s(commitsPath, curRepository->absPath, "/." PROGRAM_NAME "/commits");
	FileEntry *buf = NULL;
	int entryCount = ls(&buf, commitsPath); // list all entries in .neogit/commits/
	Commit **commits = (Commit **)malloc(sizeof(Commit *) * entryCount);
	uint commitCount = 0;
	for (int i = 0; i < entryCount; ++i)
	{
		uint64_t hash = 0;
		sscanf(getFileName(buf[i].path), "%lx", &hash); // find the hash from filename
		commits[commitCount++] = getCommit(hash);		// parse the commit file
		if (commits[commitCount - 1] == NULL)
			commitCount--;
	}
	freeFileEntry(buf, entryCount);
	if (buf)
		free(buf);
	qsort(commits, commitCount, sizeof(Commit *), __cmpCommitsByDateDescending); // Sort Commits

	// obtain list of branches
	String _branches[20] = {NULL};
	uint64_t _branchHeads[20] = {0};
	int _branch_count = listBranches(_branches, _branchHeads);

	uint printedLogCount = 0;
	for (uint i = 0; i < commitCount && printedLogCount < options.n; i++)
	{
		if (!isMatch(commits[i]->branch, options.branch))
			continue;
		else if (!isMatch(commits[i]->username, options.author))
			continue;
		else if (commits[i]->time < options.since || commits[i]->time > options.before)
			continue;
		else if (!strReplace(NULL, commits[i]->message, options.search, NULL)) // word pattern not found
			continue;

		printf(_REDB "\n*" _RST " Commit " _YELB "'%06lx'" _RST " : on branch " _YELB "'%s'" _RST, commits[i]->hash, commits[i]->branch);
		for (int j = 0; j < _branch_count; j++)
			if (_branchHeads[j] == commits[i]->hash)
				printf(_GRNB " (%s Head)" _RST, _branches[j]);
		if (commits[i]->hash == curRepository->head.hash)
			printf(" " _REDB "-> HEAD" _RST);
		printf("\n");

		char datetime[DATETIME_STR_MAX];
		strftime(datetime, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&commits[i]->time));
		char boldedMsg[STR_MAX];
		if (!strcmp(options.search, "*"))
			strcat_s(boldedMsg, _BOLD, commits[i]->message, _UNBOLD);
		else
			strReplace(boldedMsg, commits[i]->message, options.search, boldAndUnderlineText);

		printf("  Date and Time : " _BOLD "%s\n" _RST, datetime);
		printf("  Author: " _CYANB "%s <%s>" _RST "\n", commits[i]->username, commits[i]->useremail);
		printf("  Commit Message: " _CYAN "'%s'\n" _RST, boldedMsg);

		// list tags for this commit
		Tag *_tags = NULL;
		uint _tag_count = listTags(&_tags, commits[i]->hash);
		for (int j = 0; j < _tag_count; j++)
		{
			if (j == 0)
				printf("  Associated Tags: " _MAGNTA _BOLD "%s " _RST, _tags[0].tagname);
			else
				printf("/ " _MAGNTA _BOLD "%s " _RST, _tags[j].tagname);
			if (j == _tag_count - 1)
				printf("\n");
		}
		freeTagStruct(_tags, _tag_count);

		printf("  " _DIM "[" _BOLD "%u" _UNBOLD _DIM " file(s) commited]\n" _UNBOLD _RST, commits[i]->commitedFiles.len);

		printf("\n");
		printedLogCount++;
	}

	for (uint i = 0; i < commitCount; i++)
		freeCommitStruct(commits[i]);
	
	for (int j = 0; j <= _branch_count; j++)
		if (_branches[j])
			free(_branches[j]);

	if (printedLogCount == 0)
	{
		if (commitCount == 0)
			printWarning("There is no commit in your repository!");
		else
			printWarning("There is no commit matching your options.");
	}

	if (entryCount)
		free(commits);

	return ERR_NOERR;
}

int command_branch(int argc, constString argv[], bool performActions)
{
	if (argc == 1) // list all branches
	{
		// Check Syntax
		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		String branches[20];
		uint64_t headHashes[20];
		int count = listBranches(branches, headHashes);
		printf("\n");
		for (int i = 0; i < count; i++)
		{
			if (headHashes[i] == curRepository->head.hash && strcmp(curRepository->head.branch, branches[i]) == 0)
				printf(_GRNB "%s" _UNBOLD " (HEAD)\n" _RST, branches[i]);
			else if (strcmp(curRepository->head.branch, branches[i]) == 0)
				printf(_YELB "%s" _UNBOLD " (DEATACHED HEAD)\n" _RST, branches[i]);
			else
				printf("%s\n", branches[i]);

			free(branches[i]);
		}
		if (branches[count])
			free(branches[count]);
		if (count < 1)
			printError("No branch found! (An error occured!)");
		printf("\n");
		return ERR_NOERR;
	}
	else if (argc == 2) // create branch
	{
		char branch[STR_LINE_MAX];
		if (strValidate(branch, argv[1], "a-zA-Z0-9_-"))
			if (performActions)
				printWarning("There's some invalid chars in the branch name you entered. Removing them...");
		if (isEmpty(branch))
			return ERR_ARGS_MISSING;

		// Check Syntax
		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		if (getBranchHead(branch) >> 24 == 0) // branch exist
		{
			printWarning("The branch %s already exist!\n", branch);
			return ERR_ALREADY_EXIST;
		}

		// create new branch
		int res = setBranchHead(branch, curRepository->head.hash);
		if (res == ERR_NOERR)
			printf("The branch " _CYANB "\'%s\'" _RST " created successfully!\n\n", branch);
		else
			printError("An error occured when creating the branch " _BOLD "%s" _UNBOLD " .\n", branch);

		return res;
	}
	else
		return ERR_ARGS_MISSING;
}

int command_checkout(int argc, constString argv[], bool performActions)
{
	// Check Syntax
	if (!performActions)
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	constString targetStr = "HEAD";
	char branch[STR_LINE_MAX];
	uint64_t hash = 0;
	bool deatched = false;

	if (isMatch(argv[1], "HEAD"))
	{
		strcpy(branch, curRepository->head.branch);
		hash = getBranchHead(branch);
	}
	else if (isMatch(argv[1], "HEAD-*"))
	{
		uint order = 0;
		if ((sscanf(argv[1], "HEAD-%u", &order) != 1) || !order)
			return ERR_ARGS_MISSING;
		hash = getBrachHeadPrev(curRepository->head.branch, order) & 0xFFFFFF;
		if (hash == 0xFFFFFF)
		{
			printError("Unable to checkout HEAD-%u : Parent does not exist!", order);
			return ERR_NOT_EXIST;
		}
		targetStr = argv[1];
		deatched = true;
		strcpy(branch, curRepository->head.branch);
	}
	else if ((getBranchHead(argv[1]) >> 24) == 0) // branch exist
	{
		strcpy(branch, targetStr = argv[1]);
		hash = getBranchHead(branch);
	}
	else if ((sscanf(targetStr = argv[1], "%lx", &hash) == 1) && hash) // try to scan commit hash
	{
		Commit *c = getCommit(hash);
		if (c != NULL) // commit hash found!
		{
			hash = c->hash;
			strcpy(branch, c->branch);
			deatched = true;
			freeCommitStruct(c);
		}
		else
		{
			printError("The entered branch / commit hash not found!");
			return ERR_NOT_EXIST;
		}
	}
	else
	{
		printError("The entered branch / commit hash not found!");
		return ERR_NOT_EXIST;
	}

	// Check if we have any change in working tree , forbid checkout!
	if (isWorkingTreeModified())
	{
		printWarning("You have some uncommitted changes in your working tree!");
		if (curRepository->deatachedHead)
		{
			printWarning(_BOLD "YOU HAVE MADE CHANGES IN WORKING TREE WHILE YOUR HEAD IS DEATACHED!!"_UNBOLD);
			if (hash == curRepository->head.hash)
			{
				printWarning(_BOLD "REVERTING CHANGES ...\n" _UNBOLD);
				goto apply; // revert current commit
			}
			else
				printWarning(_BOLD "YOU CAN ONLY CHECKOUT TO CURRENT COMMIT (TO RESET CHANGES)!" _UNBOLD);
		}
		printError("Conflict Error!");
		return ERR_NOT_COMMITED_CHANGE_FOUND;
	}

	// If Already on requested commit or branch
	if ((strcmp(curRepository->head.branch, branch) == 0) && (curRepository->head.hash == hash))
	{
		if (!deatched)
			printf("\nAlready on " _BOLD "'%s'" _UNBOLD "\n\n", branch);
		else
			printf("\nAlready on " _BOLD "'%06lx'" _UNBOLD "\n\n", hash);
		return ERR_ALREADY_EXIST;
	}

	// check if it's a merged branch
	String merged = getMergeDestination(branch);
	if (merged)
	{
		if (!deatched)
		{
			printWarning("This is a merged branch. You cannot checkout this branch. \n");
			free((void *)merged);
			return ERR_GENERAL;
		}
		else
		{
			strcpy(branch, merged);
			free((void *)merged);
		}
	}

	// change head
	if (!deatched) // (checkout branch)
		systemf("echo branch/%s>\"%s/." PROGRAM_NAME "/HEAD\"", branch, curRepository->absPath);
	else // checkout commit id
		systemf("echo commit/%06lx:%s>\"%s/." PROGRAM_NAME "/HEAD\"", hash, branch, curRepository->absPath);
	fetchHEAD(); // fetch the head to initialize the curRepository->head

apply:
	// We must check all the tracked files
	// And get changeHead , then  compare it with head
	// if we saw a difference, change the file in working tree into its head state
	int res = applyToWorkingDir(curRepository->head.headFiles);

	if (res == ERR_NOERR)
	{
		printf("You are checked out to " _CYANB "'%s'" _RST " successfully!\n\n", targetStr);
		if (curRepository->deatachedHead)
			printWarning("You're in DEATACHED HEAD mode!! Don't Chnage working tree files!\nElse, You're responsible for any problem in your repository!!\n");
	}
	else
		printError("Error in checking out to " _BOLD "'%s'" _UNBOLD " !\n", targetStr);

	return ERR_NOERR;
}