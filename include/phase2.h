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


int command_revert(int argc, constString argv[], bool performActions);
#define CMD_REVERT_USAGE                                                                                                           \
	"\n" _BOLD "neogit revert [-m <message>] <commit-id> " _UNBOLD ":  Reverts the working tree to specified commit\n"             \
	"                                            and create new commit based on that commit."                                      \
	"\n" _BOLD "neogit revert [-m <message>] HEAD-n " _UNBOLD " : Reverts the working tree to n previous state and create commit." \
	"\n" _BOLD "neogit revert -n <commit-id>|HEAD-n " _UNBOLD " : Only revert the working tree to specified state.\n"


int command_tag(int argc, constString argv[], bool performActions);
#define CMD_TAG_USAGE                                                                                                                        \
	"\n" _BOLD "neogit tag -a <tag-name> [-m <message>] [-c <commit-id>] [-f] " _UNBOLD ":  Creates a tag with specific name and message.\n" \
	"                                                                 If commit-id is not provided, creates tag for current HEAD Commit.\n"  \
	"                                                                 Switch -f is for force overwriting.\n"                                 \
	"\n" _BOLD "neogit tag                 " _UNBOLD " : List all tags (sorted by name alphabetically ascending)"                            \
	"\n" _BOLD "neogit tag show <tag-name> " _UNBOLD " : Show information of tag with specified tag-name.\n"


int command_grep(int argc, constString argv[], bool performActions);
#define CMD_GREP_USAGE                                                                                                                             \
	"\n" _BOLD "neogit grep -f <file> -p <word> [<options>] " _UNBOLD ": find a word pattern (probably wildcard included) in the specified file\n" \
	"                                              and print the lines include those matches."                                                     \
	"\nOptions : "                                                                                                                                 \
	"\n   " _BOLD "-c <commit-id>" _UNBOLD "\t Performs search in specified  commit."                                                              \
	"\n   " _BOLD "-n " _UNBOLD "\t\t\t Show line numbers for each matching line.\n"


int command_diff(int argc, constString argv[], bool performActions);
#define CMD_DIFF_USAGE                                                                                                                                           \
	"\n" _BOLD "neogit diff -f <file1> <file2> [-line1 <begin-end>] [-line2 <begin-end>] " _UNBOLD ":  Show differences between file1 and file2. (line-based)\n" \
	"                                                                         (if line bounds provided, compare the bounded line numbers)\n"                     \
	"\n" _BOLD "neogit diff -c <commit-id-1> <commit-id-2> " _UNBOLD ":  Show differnces between two commits.\n"                                                 \
	"                                            (and perfroms diff commands on file pairs - between two commits)\n"


int command_merge(int argc, constString argv[], bool performActions);
#define CMD_MERGE_USAGE                                                                                                         \
	"\n" _BOLD "neogit merge -b <branch-to-merge> [<base-branch>] " _UNBOLD ":  Merge the given branch into the base branch.\n" \
	"                                                     (if base-branch is not provided, merge the given\n"                   \
	"                                                      branch to current HEAD.)\n"

#endif