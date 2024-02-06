/*******************************
 *         phase2.h            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#ifndef __PHASE_2_H__
#define __PHASE_2_H__

#include "neogit.h"

/**
 * @brief Reverts the working tree to a specified commit and optionally creates a new commit.
 *
 * This function reverts the working tree to the state of the specified commit. If the "-n" switch is provided,
 * it only reverts the working tree without creating a new commit. If the "-m" switch is provided along with
 * a commit message, it creates a new commit with the reverted changes and the specified commit message.
 *
 * @param argc          The number of arguments.
 * @param argv          The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or only check syntax.
 * @return              An error code indicating the result of the operation.
 */
int command_revert(int argc, constString argv[], bool performActions);
#define CMD_REVERT_USAGE                                                                                                           \
	"\n" _BOLD "neogit revert [-m <message>] <commit-id> " _UNBOLD ":  Reverts the working tree to specified commit\n"             \
	"                                            and create new commit based on that commit."                                      \
	"\n" _BOLD "neogit revert [-m <message>] HEAD-n " _UNBOLD " : Reverts the working tree to n previous state and create commit." \
	"\n" _BOLD "neogit revert -n <commit-id>|HEAD-n " _UNBOLD " : Only revert the working tree to specified state.\n"

/**
 * @brief Manages tags in the repository.
 * This function provides functionality to list, show, and create tags in the repository.
 *
 * @note - neogit tag -a <tag-name> [-m <message>] [-c <commit-id>] [-f] : Creates a tag with a specific name and message.
 *   If commit-id is not provided, it creates a tag for the current HEAD commit. The switch -f is for force overwriting.
 * @note - neogit tag : Lists all tags (sorted by name alphabetically ascending).
 * @note - neogit tag show <tag-name> : Shows information about the tag with the specified tag-name.
 *
 * @param argc          The number of arguments.
 * @param argv          The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or only check syntax.
 * @return              An error code indicating the result of the operation.
 */
int command_tag(int argc, constString argv[], bool performActions);
#define CMD_TAG_USAGE                                                                                                                        \
	"\n" _BOLD "neogit tag -a <tag-name> [-m <message>] [-c <commit-id>] [-f] " _UNBOLD ":  Creates a tag with specific name and message.\n" \
	"                                                                 If commit-id is not provided, creates tag for current HEAD Commit.\n"  \
	"                                                                 Switch -f is for force overwriting.\n"                                 \
	"\n" _BOLD "neogit tag                 " _UNBOLD " : List all tags (sorted by name alphabetically ascending)"                            \
	"\n" _BOLD "neogit tag show <tag-name> " _UNBOLD " : Show information of tag with specified tag-name.\n"


/**
 * @brief Searches for a word pattern in the specified file and prints lines including the matches.
 *
 * This function allows finding a word pattern (possibly including a wildcard) in the specified file and printing
 * the lines that include those matches. Various options such as searching in a specific commit and showing line numbers
 * are available.
 *
 * @param argc          The number of arguments.
 * @param argv          The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or only check syntax.
 * @return              An error code indicating the result of the operation.
 */
int command_grep(int argc, constString argv[], bool performActions);
#define CMD_GREP_USAGE                                                                                                                             \
	"\n" _BOLD "neogit grep -f <file> -p <word> [<options>] " _UNBOLD ": find a word pattern (probably wildcard included) in the specified file\n" \
	"                                              and print the lines include those matches."                                                     \
	"\nOptions : "                                                                                                                                 \
	"\n   " _BOLD "-c <commit-id>" _UNBOLD "\t Performs search in specified  commit."                                                              \
	"\n   " _BOLD "-n " _UNBOLD "\t\t\t Show line numbers for each matching line.\n"


/**
 * @brief Shows differences between files or commits.
 *
 * This function supports two modes of operation:
 * 1. `neogit diff -f <file1> <file2> [-line1 <begin-end>] [-line2 <begin-end>]`: Shows differences between file1 and file2, optionally comparing specific lines.
 * 2. `neogit diff -c <commit-id-1> <commit-id-2>`: Shows differences between two commits and performs diff commands on file pairs.
 *
 * Usage:
 * - `neogit diff -f <file1> <file2> [-line1 <begin-end>] [-line2 <begin-end>]`: Shows differences between file1 and file2 (line-based).
 *    If line bounds are provided, it compares the bounded line numbers.
 * - `neogit diff -c <commit-id-1> <commit-id-2>`: Shows differences between two commits and performs diff commands on file pairs.
 *
 * Options:
 * - `-line1 <begin-end>`: Specifies line bounds for file1 (optional).
 * - `-line2 <begin-end>`: Specifies line bounds for file2 (optional).
 *
 * @param argc          The number of arguments.
 * @param argv          The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or only check syntax.
 * @return              An error code indicating the result of the operation.
 */
int command_diff(int argc, constString argv[], bool performActions);
#define CMD_DIFF_USAGE                                                                                                                                           \
	"\n" _BOLD "neogit diff -f <file1> <file2> [-line1 <begin-end>] [-line2 <begin-end>] " _UNBOLD ":  Show differences between file1 and file2. (line-based)\n" \
	"                                                                         (if line bounds provided, compare the bounded line numbers)\n"                     \
	"\n" _BOLD "neogit diff -c <commit-id-1> <commit-id-2> " _UNBOLD ":  Show differnces between two commits.\n"                                                 \
	"                                            (and perfroms diff commands on file pairs - between two commits)\n"

/**
 * @brief Merges the given branch into the base branch.
 *
 * This function performs a merge operation by combining changes from the specified branch (`mergingBr`) into the base branch (`baseBr`).
 * If `base-branch` is not provided, the merge is performed into the current HEAD branch.
 *
 * Usage:
 * - `neogit merge -b <branch-to-merge> [<base-branch>]`: Merges the given branch into the base branch.
 *   If the base branch is not provided, it merges the branch into the current HEAD branch.
 *
 * @param argc          The number of arguments.
 * @param argv          The array of command-line arguments.
 * @param performActions A boolean indicating whether to perform the actions or only check syntax.
 * @return              An error code indicating the result of the operation.
 */
int command_merge(int argc, constString argv[], bool performActions);
#define CMD_MERGE_USAGE                                                                                                         \
	"\n" _BOLD "neogit merge -b <branch-to-merge> [<base-branch>] " _UNBOLD ":  Merge the given branch into the base branch.\n" \
	"                                                     (if base-branch is not provided, merge the given\n"                   \
	"                                                      branch to current HEAD.)\n"

#endif