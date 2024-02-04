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
#define CMD_REVERT_USAGE                                                                                               \
	"\n" _BOLD "neogit revert [-m <message>] <commit-id> " _UNBOLD ":  Reverts the working tree to specified commit\n" \
	"                                            and create new commit based on that commit."                          \
	"\n" _BOLD "neogit revert [-m <message>] HEAD-n " _UNBOLD " : Reverts the working tree to n previous state and create commit." \
	"\n" _BOLD "neogit revert -n <commit-id>|HEAD-n " _UNBOLD " : Only revert the working tree to specified state.\n"

int command_tag(int argc, constString argv[], bool performActions);
#define CMD_TAG_USAGE ""

int command_grep(int argc, constString argv[], bool performActions);
#define CMD_GREP_USAGE ""

int command_diff(int argc, constString argv[], bool performActions);
#define CMD_DIFF_USAGE ""

int command_merge(int argc, constString argv[], bool performActions);
#define CMD_MERGE_USAGE ""

#endif