#ifndef __PHASE_2_H__
#define __PHASE_2_H__

#include "neogit.h"


int command_revert(int argc, constString argv[], bool performActions);
#define CMD_REVERT_USAGE ""



int command_tag(int argc, constString argv[], bool performActions);
#define CMD_TAG_USAGE ""



int command_stash(int argc, constString argv[], bool performActions);
#define CMD_STASH_USAGE ""



int command_grep(int argc, constString argv[], bool performActions);
#define CMD_GREP_USAGE ""



int command_diff(int argc, constString argv[], bool performActions);
#define CMD_DIFF_USAGE ""



int command_merge(int argc, constString argv[], bool performActions);
#define CMD_MERGE_USAGE ""

#endif