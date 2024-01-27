#ifndef __PHASE_1_H__
#define __PHASE_1_H__

#include "header.h"

int command_init(int argc, constString argv[], bool performActions);
#define CMD_INIT_USAGE ""

/*
 * This function handles the 'config' command. It allows users to set or remove configurations,
 * including both user-specific and alias configurations. The configurations can be set globally
 * using the '--global' flag or locally within a specific repository.
 * Using -R switch , we can remove a global/local config.
 *
 * Parameters:
 * - argc: The number of command-line arguments.
 * - argv: An array of command-line argument strings.
 * - performActions: A boolean indicating whether to perform the specified actions or just check syntax.
 *
 * Returns:
 * - ERR_ARGS_MISSING: Indicates missing or invalid arguments.
 * - ERR_NOREPO: Indicates that the repository is not initialized when attempting a local configuration.
 * - ERR_NOERR: Indicates successful execution with no errors.
 * - Various error codes for specific failures during set or remove operations.
 */
int command_config(int argc, constString argv[], bool performActions);
#define CMD_CONFIG_USAGE "Set a config: " _SGR_BOLD "neogit config [--global] <key> <value>\n" _SGR_NORM \
						 "Remove a config : " _SGR_BOLD "neogit config [--global] -R <key>\n" _SGR_NORM  \
						 "Valid keys are : user.* / alias.*"


int command_add(int argc, constString argv[], bool performActions);
#define CMD_ADD_USAGE ""


int command_reset(int argc, constString argv[], bool performActions);
#define CMD_RESET_USAGE ""


int command_status(int argc, constString argv[], bool performActions);
#define CMD_STATUS_USAGE ""


int command_commit(int argc, constString argv[], bool performActions);
#define CMD_COMMIT_USAGE ""


int command_set(int argc, constString argv[], bool performActions);
#define CMD_SET_USAGE ""


int command_replace(int argc, constString argv[], bool performActions);
#define CMD_REPLACE_USAGE ""


int command_remove(int argc, constString argv[], bool performActions);
#define CMD_REMOVE_USAGE ""


int command_log(int argc, constString argv[], bool performActions);
#define CMD_LOG_USAGE ""


int command_branch(int argc, constString argv[], bool performActions);
#define CMD_BRANCH_USAGE ""


int command_checkout(int argc, constString argv[], bool performActions);
#define CMD_CHECKOUT_USAGE ""

#endif