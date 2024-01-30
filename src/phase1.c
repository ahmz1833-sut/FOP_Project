#include "phase1.h"
#include "hash.h"

extern String curWorkingDir;      // Declared in neogit.c
extern Repository *curRepository; // Declared in neogit.c

void printTree(FileEntry *root, uint curDepth, int (*listFunction)(FileEntry **, constString), void (*printFunction)(FileEntry *))
{
    bool isLastBefore[16];
    // Extract is last before (in tree prinitng) from bits  of flags
    for (int i = 4; i < 20; i++)
        isLastBefore[i - 4] = curDepth & (1 << i);

    curDepth &= 0xF;   // Clear all but lower 4 bits (real curDepth)
    if (curDepth == 0) // base of recursive
        return;

    static int maxDepth = 0;
    if (maxDepth < curDepth) // First call
    {
        maxDepth = curDepth;
        // calculate relative path of root based on repository path
        FileEntry fe = getFileEntry(root->path, curRepository->absPath);
        printFunction(&fe);
        freeFileEntry(&fe, 1);
    }

    FileEntry *array;
    uint num = listFunction(&array, root->path);
    if (!num)
        return;

    for (int i = 0; i < num; i++)
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

        printFunction(&array[i]);
        if (array[i].isDir && curDepth > 1)
        {
            uint passedInt = curDepth - 1;
            for (int i = 4; i < 20; i++)
                passedInt |= (isLastBefore[i - 4] ? (1 << i) : 0);
            printTree(&array[i], passedInt, listFunction, printFunction);
        }
    }
    freeFileEntry(array, num);
    free(array);
}

void printStagedStatus(FileEntry *element)
{
    if (element->isDir)
    {
        printf(_BLU _BOLD "%s" _UNBOLD " (dir)\n" _RST, getFileName(element->path));
    }
    else
    {
        printf("%s\n", getFileName(element->path));
    }
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
        withString(neogitFolder, strConcat(curWorkingDir, "/." PROGRAM_NAME))
            mkdir(neogitFolder, 0755);
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

    tryWithString(key, malloc(strlen(argv[keyArgIndex])), ({ return ERR_MALLOC; }), ({ return _ERR; }))
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
        tryWithString(value, malloc(strlen(argv[keyArgIndex + 1])), throw(ERR_MALLOC), throw(_err))
        {
            invalid += strValidate(value, argv[keyArgIndex + 1], "a-zA-Z0-9.@!~( )_-");
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

        FileEntry root = getFileEntry(".", NULL);
        printTree(&root, atoi(argv[2]), ls, printStagedStatus);
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
        backupStagingArea();

    while (argc--)
    {
        constString argument = *(argv++);
        char dest[PATH_MAX];
        uint illegal = strValidate(dest, argument, VALID_CHARS);
        strtrim(dest);

        if (illegal || !*dest)
        {
            printWarning("File or directory " _BOLD "\"%s\"" _UNBOLD " has illegal characters! It has been ignored.", argument);
            throw(0);
        }

        FileEntry *entries = NULL;
        int result = ls(&entries, dest); // TODO: Change to lsWDirAndHead()
        if (result == -1)                // error
            printError("Error! " _BOLD "\"%s\"" _UNBOLD " : No such file or directory!", argument);
        else if (result == -2) // add file
        {
            FileEntry fileE = getFileEntry(dest, curRepository->absPath); // TODO: Chnage to handle notexist files

            if (fileE.path == NULL) // TODO: Handle deleted file
                printError("Error in adding File " _BOLD "\"%s\"" _UNBOLD, argument);
            else
            {
                // .gitignore patterns are handled here.
                if (isGitIgnore(&fileE))
                    continue;
                int res = 0;
                trackFile(fileE.path);
                if (isChangedStaging(fileE.path))
                {
                    res = addToStage(&fileE);
                    if (res == ERR_NOERR)
                        printf("File Added to stage: " _CYAN "%s " _YEL "%s\n"_RST, fileE.path, getStagedFile(fileE.path)->hashedPath);
                    else
                        printError("Error! in adding file: " _BOLD "%s" _UNBOLD ".\n", fileE.path);
                }
                else
                {
                    printf(_DIM "File already staged: %s\n" _RST, fileE.path);
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
        else
            printError("Error : in restoring stage backup.");
        return res;
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

    while (argc--)
    {
        constString argument = *(argv++);
        char dest[PATH_MAX];
        uint illegal = strValidate(dest, argument, VALID_CHARS);
        strtrim(dest);

        if (illegal || !*dest)
        {
            printWarning("File or directory " _BOLD "\"%s\"" _UNBOLD " has illegal characters! It has been ignored.", argument);
            throw(0);
        }

        FileEntry *entries = NULL;
        int result = ls(&entries, dest); // TODO: Change to lsWDirAndHead()
        if (result == -1)                // error
            printError("Error! " _BOLD "\"%s\"" _UNBOLD " : No such file or directory!", argument);
        else if (result == -2) // add file
        {
            FileEntry fileE = getFileEntry(dest, curRepository->absPath); // TODO: Chnage to handle notexist files

            if (fileE.path == NULL) // TODO: Handle deleted file
                printError("Error in adding File " _BOLD "\"%s\"" _UNBOLD, argument);
            else
            {
                int res = 0;
                StagedFile *sf = getStagedFile(fileE.path);
                if (sf)
                {
                    res = removeFromStage(sf);
                    if (res == ERR_NOERR)
                        printf("File Added to stage: " _CYAN "%s " _YEL "%s\n"_RST, fileE.path, getStagedFile(fileE.path)->hashedPath);
                    else
                        printError("Error! in adding file: " _BOLD "%s" _UNBOLD ".\n", fileE.path);
                }
                else
                {
                    printf(_DIM "File already staged: %s\n" _RST, fileE.path);
                }
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
                new_argv[new_argc++] = entries[i].path;

            // Add childs recursive
            command_add(new_argc, (constString *)new_argv, true);
            freeFileEntry(entries, result);
            if (result)
                free(entries);
        }
    }
    return ERR_NOERR;
}

int command_status(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_commit(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_set(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_replace(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_remove(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_log(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_branch(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}

int command_checkout(int argc, constString argv[], bool performActions)
{
    // Check Syntax

    // Perform Acions if required
    if (performActions)
    {
        if (!curRepository)
            return ERR_NOREPO;
    }
}