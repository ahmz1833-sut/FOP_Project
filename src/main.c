#include "header.h"
#include "install.h"
#include "phase1.h"
#include "phase2.h"

// #define __DEBUG_MODE__ "neogit config --global -R user.name"

const Command cmds[] = {
    {"init", 2, 2, command_init, CMD_INIT_USAGE},
    {"config", 4, 5, command_config, CMD_CONFIG_USAGE},
    {"add", 3, 0, command_add, CMD_ADD_USAGE},
    {"reset", 3, 0, command_reset, CMD_RESET_USAGE},
    {"status", 2, 2, command_status, CMD_STATUS_USAGE},
    {"commit", 4, 4, command_commit, CMD_COMMIT_USAGE},
    {"set", 6, 6, command_set, CMD_SET_USAGE},
    {"replace", 6, 6, command_replace, CMD_REPLACE_USAGE},
    {"remove", 4, 4, command_remove, CMD_REMOVE_USAGE},
    {"log", 2, 4, command_log, CMD_LOG_USAGE},
    {"branch", 2, 3, command_branch, CMD_BRANCH_USAGE},
    {"checkout", 3, 3, command_checkout, CMD_CHECKOUT_USAGE},
    {"revert", 3, 7, command_revert, CMD_REVERT_USAGE},
    {"tag", 2, 9, command_tag, CMD_TAG_USAGE},
    {"tree", 2, 2, command_tree, CMD_TREE_USAGE},
    {"stash", 3, 5, command_stash, CMD_STASH_USAGE},
    {"pre-commit", 2, 0, command_precommit, CMD_PRECOMMIT_USAGE},
    {"grep", 6, 9, command_grep, CMD_GREP_USAGE},
    {"diff", 5, 9, command_diff, CMD_DIFF_USAGE},
    {"merge", 5, 6, command_merge, CMD_MERGE_USAGE},
    {NULL, 0, 0, NULL, NULL}}; // End of Commands list

/**
 * @brief Displays a welcome message and lists valid commands and configured aliases.
 *
 * The Welcome function prints a welcome message to the console, listing valid commands and configured aliases.
 * It uses ANSI escape codes for colored output.
 * Valid commands and configured aliases are retrieved and displayed in a readable format.
 */
void Welcome();

#ifdef __DEBUG_MODE__
int main()
{
    constString argv[20];
    uint argc = tokenizeString(strDup(__DEBUG_MODE__), " ", (String*)argv);
#else
int main(int argc, constString argv[])
{
#endif

    // Fetch the current working directory and find the repo in parents if found
    // Put these into global variables in Global variables in neogit.c
    extern String curWorkingDir; // Declared in neogit.c
    curWorkingDir = getcwd(NULL, PATH_MAX);
    extern String curRepoPath; // Declared in neogit.c
    curRepoPath = obtainRepository(curWorkingDir);

    if (checkArgument(1, "--uninstall")) // Uninstall Command
        exit(promptUninstallation());

    else if (checkArgument(1, "--install")) // Install Command (for upgrade)
        exit(promptInstallation(argv[0]));

    if (!IS_INSTALLED()) // NeoGIT is not installed!!
        promptInstallation(argv[0]);

    // Process the command! (and perform it)
    return process_command(argc, argv, true);
}

/* perform the command or check the command syntax */
int process_command(int argc, constString argv[], bool performActions)
{
    // Check if the program name is valid
    if (!checkArgument(0, PROGRAM_NAME))
        return ERR_COMMAND_INVALID;

    // Check the command
    for (int i = 0; cmds[i].key != NULL; i++)
    {
        if (checkArgument(1, cmds[i].key))
        {
            // Display help message if "--help" is provided
            if (checkArgumentPure(2, "--help"))
            {
                if (performActions)
                    printf(_SGR_CYANF "Usage : \n%s\n"_SGR_RESET, cmds[i].usageHelp);
                return ERR_NOERR;
            }
            // Execute the command function if arguments are valid
            else if (argc >= cmds[i].minArgc && (!cmds[i].maxArgc || argc <= cmds[i].maxArgc) && !(cmds[i].function(argc - 1, argv + 1, false)))
                return cmds[i].function(argc - 1, argv + 1, performActions);

            // Display an error message if arguments are missing or invalid
            if (performActions)
            {
                printError("Error! Missing Argument or Invalid Command!");
                printf(_SGR_CYANF "Usage : \n%s\n"_SGR_RESET, cmds[i].usageHelp);
            }
            return ERR_ARGS_MISSING;
        }
    }

    if (argc == 2) // Check if command is an alias
    {
        String alias = strConcat("alias.", argv[1]);
        String aliasCommand = getConfig(alias);
        free(alias);
        if (aliasCommand)
        {
            String alias_argv[20];
            uint alias_argc = tokenizeString(aliasCommand, " ", alias_argv);
            return process_command(alias_argc, (constString *)alias_argv, performActions);
        }
    }

    // Display welcome message and usage help for an invalid or empty command
    if (performActions)
        Welcome();
    return ERR_COMMAND_INVALID;
}

void Welcome()
{
    printf(_SGR_YELLOWF "Welcome to the neogit!\n"_SGR_DEF_FG
                        "Valid commands are listed below: \n");
    for (int i = 0; cmds[i].key != NULL; i++)
        printf(_SGR_YELLOWF "|" _SGR_DEF_FG " %s ", cmds[i].key);
    printf("\n" _SGR_RESET);

    // Retrieve configured aliases
    String aliases[50];
    uint aliasNum = getAliases(aliases);

    // Display configured aliases if available
    if (aliasNum)
        printf("\nConfigured Aliases : \n");
    for (int i = 0; i < aliasNum; i++)
        printf(_SGR_CYANF "%s" _SGR_REDF " -> " _SGR_DEF_FG "%s\n", aliases[i], getConfig(strConcat("alias.", aliases[i])));
}