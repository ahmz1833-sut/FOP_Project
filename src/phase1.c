#include "phase1.h"

extern String curWorkingDir; // Declared in neogit.c
extern String curRepoPath;   // Declared in neogit.c

int command_init(int argc, constString argv[], bool performActions)
{
    if (performActions)
    {
        // Repo already exist
        if (curRepoPath)
        {
            printError("The repository is already initialized in " _SGR_BOLD "\"%s\"" _SGR_NORM ".", curRepoPath);
            return ERR_ALREADY_EXIST;
        }
        
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
    if (performActions && !globalFlagIndex && !curRepoPath)
        perrExit(ERR_NOREPO, "Repository is not initialized!!");

    // Extract key and validate it
    String key = malloc(strlen(argv[keyArgIndex]));
    uint invalid = strValidate(key, argv[keyArgIndex], "a-zA-Z0-9._-");
    if (isEmpty(key))
        return ERR_ARGS_MISSING;

    if (checkAnyArgument("-R")) // remove config
    {
        if (!performActions)
            return ERR_NOERR;
        int res = removeConfig(key, globalFlagIndex);
        if (res == ERR_NOERR)
            printf("The config [" _SGR_YELLOWF "%s" _SGR_DEF_FG "] successfully removed from %s!\n", key, globalFlagIndex ? "global" : "local");
        else if (res == ERR_CONFIG_NOTFOUND)
            printError("The config [%s] not found in %s!", key, globalFlagIndex ? "global" : "local");
        else
            perrExit(res, "ERROR! In remove config [%s] from %s", key, globalFlagIndex ? "global" : "local");

        return res;
    }

    // Extract value and validate it
    String value = malloc(strlen(argv[keyArgIndex + 1]));
    invalid += strValidate(value, argv[keyArgIndex + 1], "a-zA-Z0-9.@!~( )_-");
    if (invalid)
        if (performActions)
            printWarning("You're Entered at least one invalid character in value. It will be removed.");
    if (isEmpty(value))
        return ERR_ARGS_MISSING;

    // Check and process alias commands
    if (isAlias)
    {
        String alias_argv[20];
        String commandDuplicate = strDup(value);
        uint alias_argc = tokenizeString(commandDuplicate, " ", alias_argv);
        // Check the command syntax
        int result = process_command(alias_argc, (constString *)alias_argv, false);
        free(commandDuplicate);
        if (result != ERR_NOERR)
            perrExit(result, "Error in set alias: Command [%s] is incorrect!", value);
    }

    //  if we don't have to perform actions, return OK (syntax OK)
    if (!performActions)
        return ERR_NOERR;

    // Set the configuration value
    time_t now = time(NULL);
    uint errcode = setConfig(key, value, now, globalFlagIndex);
    if (errcode == ERR_NOERR)
        printf("The %s config [" _SGR_CYANF "%s" _SGR_DEF_FG "] successfully set to " _SGR_CYANF "%s" _SGR_DEF_FG " at time %ld.\n" _SGR_RESET,
               globalFlagIndex ? "global" : "local", key, value, now);
    else
        perrExit(errcode, "ERROR! In set config [%s] to [%s]", key, value);

    free(key);
    free(value);
}

int command_add(int argc, constString argv[], bool performActions)
{
    printf("Config [" _SGR_CYANF "%s" _SGR_DEF_FG "] is " _SGR_CYANF "88" _SGR_RESET "\n", argv[1]);
}

int command_reset(int argc, constString argv[], bool performActions)
{
}

int command_status(int argc, constString argv[], bool performActions)
{
}

int command_commit(int argc, constString argv[], bool performActions)
{
}

int command_set(int argc, constString argv[], bool performActions)
{
}

int command_replace(int argc, constString argv[], bool performActions)
{
}

int command_remove(int argc, constString argv[], bool performActions)
{
}

int command_log(int argc, constString argv[], bool performActions)
{
}

int command_branch(int argc, constString argv[], bool performActions)
{
}

int command_checkout(int argc, constString argv[], bool performActions)
{
}