#ifndef __PHASE_2_H__
#define __PHASE_2_H__

#include "header.h"

#define CMD_REVERT_USAGE ""
#define CMD_TAG_USAGE ""
#define CMD_TREE_USAGE ""
#define CMD_STASH_USAGE ""
#define CMD_PRECOMMIT_USAGE ""
#define CMD_GREP_USAGE ""
#define CMD_DIFF_USAGE ""
#define CMD_MERGE_USAGE ""

int command_revert(int argc, constString argv[], bool performActions);
int command_tag(int argc, constString argv[], bool performActions);
int command_tree(int argc, constString argv[], bool performActions);
int command_stash(int argc, constString argv[], bool performActions);
int command_precommit(int argc, constString argv[], bool performActions);
int command_grep(int argc, constString argv[], bool performActions);
int command_diff(int argc, constString argv[], bool performActions);
int command_merge(int argc, constString argv[], bool performActions);

#endif