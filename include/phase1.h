/*******************************
 *         phase1.h            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#ifndef __PHASE_1_H__
#define __PHASE_1_H__

#include "neogit.h"

// Declare an struct for command 'log' options
typedef struct _log_options_t
{
	uint n;
	constString branch;
	constString author;
	time_t since;
	time_t before;
	constString search;
} LogOptions;

/**
 * @brief Initialize a new Git repository.
 *
 * This function initializes a new Git repository in the current working directory.
 * It creates the necessary directory structure and files for Neogit to function.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the initialization actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_init(int argc, constString argv[], bool performActions);
#define CMD_INIT_USAGE _BOLD "Initialize a repository : neogit init\n" _UNBOLD

/**
 * @brief Handle Git configurations, including alias and user configurations.
 *
 * This function handles the 'config' command. It allows users to set or remove configurations,
 * including both user-specific and alias configurations. The configurations can be set globally
 * using the '--global' flag or locally within a specific repository.
 * Using -R switch , we can remove a global/local config.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the configuration actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_config(int argc, constString argv[], bool performActions);
#define CMD_CONFIG_USAGE "Set a config: " _BOLD PROGRAM_NAME " config [--global] <key> <value>\n" _UNBOLD \
						 "Remove a config : " _BOLD PROGRAM_NAME " config [--global] -R <key>\n" _UNBOLD  \
						 "Valid keys are : user.* / alias.*\n"

/**
 * @brief Add files to the staging area or list, stage, or undo changes.
 *
 * This function handles the "add" command. It allows adding files to the staging area, listing files with staging state,
 * restaging modified tracked files, or adding files and directories recursively to the staging area.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the add actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_add(int argc, constString argv[], bool performActions);
#define CMD_ADD_USAGE "Add file(s)/folder(s) to stage : " _BOLD "neogit add [-f] <file> [file2 file3 ...]\n" _UNBOLD \
					  "Restage all modified tracked file(s) : " _BOLD "neogit add -redo\n" _UNBOLD                   \
					  "Show staging status of working tree : " _BOLD "neogit add -n <depth>\n" _UNBOLD

/**
 * @brief Reset or undo changes in the working directory or staging area.
 *
 * This function handles the "reset" command. It allows resetting or undoing changes in the working directory or staging area.
 * The function supports options like "-undo" to undo last opertion was performed by 'add' command. (Up to 10 level)
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the reset actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_reset(int argc, constString argv[], bool performActions);
#define CMD_RESET_USAGE "Unstage file(s)/folder(s) : " _BOLD "neogit [-f] <file> [file2 file3 ...]\n" _UNBOLD \
						"Undo last 'add' operation : " _BOLD "neogit reset -undo\n" _UNBOLD

/**
 * @brief Display the status of the working directory.
 *
 * This function handles the "status" command. It displays information about the current branch and the status of the working tree.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the status actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_status(int argc, constString argv[], bool performActions);
#define CMD_STATUS_USAGE "Show status of working tree and changed files: " _BOLD "neogit status\n" _UNBOLD      \
						 "In printed tree::  A : Added / D : Deleted / M : Modified / T : permission-changed\n" \
						 "Flag + means that change is staged, - means it is not.\n"

/**
 * @brief Perform a Git commit operation.
 *
 * This function handles the "commit" command in Git. It creates a commit with the specified message or uses a shortcut key for the message.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the commit actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_commit(int argc, constString argv[], bool performActions);
#define CMD_COMMIT_USAGE "Perform a commit : " _BOLD "neogit commit [-m <message>]|[-s <shortcut-key>]\n" _UNBOLD \
						 "(Max Commit Message Length is 72 characters)\n"

/**
 * @brief Set or replace a shortcut message.
 *
 * This function handles the "shortcutmsg" command in Git. It sets or replaces a shortcut message associated with a key.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_shortcutmsg(int argc, constString argv[], bool performActions);
/**
 * @brief Remove a shortcut message.
 *
 * This function handles the "remove" command in Git. It removes a shortcut message associated with a key.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_remove(int argc, constString argv[], bool performActions);
#define CMD_SHORTCUT_USAGE "Set a shortcut message : " _BOLD "neogit set -m <message> -s <shortcut-key>" _UNBOLD "\n"         \
						   "Replace a shortcut message : " _BOLD "neogit replace -m <message> -s <shortcut-key>" _UNBOLD "\n" \
						   "Remove a shortcut message : " _BOLD "neogit remove -s <shortcut-key>\n" _UNBOLD

/**
 * @brief Log command to display commit history.
 *
 * This function handles the "log" command in Git. It displays the commit history based on the provided options.
 * @note - If no argument is provided, it will show all commits
 * @note - option -n <number> : show last n commits
 * @note - option -since <datetime>  : show commits since datetime
 * @note - option -before <datetime> : show commits before datetime
 * @note - option -author <string> : Show only commits from author
 * @note - option -branch <branchname> : Show only commits belongs to spicific branch
 * @note - option -search <wordpattern> : Search for word pattern in messages
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_log(int argc, constString argv[], bool performActions);
#define CMD_LOG_USAGE "Show commits with specified filter options.\n" _BOLD "neogit log [options]\n\n" _UNBOLD           \
					  "Options:"                                                                                         \
					  "\n" _BOLD "-n <number>       " _UNBOLD ": Show last n commits"                                    \
					  "\n" _BOLD "-branch <branch>  " _UNBOLD ": Show only commits belongs to spicific branch"           \
					  "\n" _BOLD "-since <datetime> " _UNBOLD ": Show commits since yyyy-mm-dd or \"yyyy-mm-dd HH:mm\""  \
					  "\n" _BOLD "-before <date>    " _UNBOLD ": Show commits before yyyy-mm-dd or \"yyyy-mm-dd HH:mm\"" \
					  "\n" _BOLD "-author <name>    " _UNBOLD ": Filter by author."                                      \
					  "\n" _BOLD "-search <word>    " _UNBOLD ": Search for word/words in commit messages.\n"

/**
 * @brief Branch command to list or create branches.
 *
 * This function handles the "branch" command in Git. It can list all branches or create a new branch.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_branch(int argc, constString argv[], bool performActions);
#define CMD_BRANCH_USAGE "Show a list of all branches : " _BOLD "neogit branch\n" _UNBOLD \
						 "Create a new branch : " _BOLD "neogit branch <new-branch>\n" _UNBOLD

/**
 * @brief Checkout command to switch branches or commits.
 *
 * This function handles the "checkout" command in Git. It can switch branches or commits.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or not.
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int command_checkout(int argc, constString argv[], bool performActions);
#define CMD_CHECKOUT_USAGE "Checkout to a branch : " _BOLD "neogit checkout <branch>" _UNBOLD "\n" \
						   "Checkout to HEAD of current branch : " _BOLD "neogit checkout HEAD" _UNBOLD "\n" \
						   "Checkout to n previous commits from HEAD : " _BOLD "neogit checkout HEAD-n" _UNBOLD "\n" \
						   "Checkout to a specific commit : " _BOLD "neogit checkout <commit>"  _UNBOLD "\n"

#endif