#ifndef __NEOGIT_H__
#define __NEOGIT_H__

#include "common.h"
#include "string_funcs.h"
#include "file_system.h"
#include "hash.h"

#define PROGRAM_NAME "neogit"

typedef struct _command_t
{
	constString key;
	uint minArgc;
	uint maxArgc;
	int (*function)(int, constString[], bool);
	constString usageHelp;
} Command;

typedef struct _staged_file_t
{
	FileEntry file;
	char hashStr[10]; // 10 digit hash
} StagedFile;

typedef struct _staged_file_array_t
{
	StagedFile *arr;
	uint len;
} StagedFileArray;

typedef struct _head_t
{
	uint64_t hash;
	String branch;
	StagedFileArray headFiles;
} HEAD;

typedef struct _commit_t
{
	uint64_t hash;
	String username;
	String useremail;
	time_t time;
	String message;
	String branch;
	StagedFileArray commitedFiles;
	StagedFileArray headFiles;
	uint64_t prev;
	uint64_t mergedCommit;
} Commit;

typedef struct _repository_t
{
	String absPath;
	StagedFileArray stagingArea;
	HEAD head;
	bool deatachedHead;
} Repository;

typedef enum _change_status_t
{
	NOT_CHANGED = 0,
	MODIFIED,
	ADDED,
	DELETED,
	PERM_CHANGED
} ChangeStatus;

int processTree(FileEntry *root, uint curDepth, int (*listFunction)(FileEntry **, constString), bool print_tree, void (*callbackFunction)(FileEntry *));

/**
 * @brief Process and execute a command or check the command syntax.
 *
 * The process_command function takes the command-line arguments and performs various checks.
 * It iterates through the available commands, checks if the provided command is valid, and executes
 * the corresponding function or displays an error message.
 *
 * # Note : This function declared in main.c
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @param performActions A boolean indicating whether to perform actions or only check the syntax.
 *
 * @return An integer representing the error code. Returns ERR_NOERR if successful, otherwise an error code.
 */
int process_command(int argc, constString argv[], bool performActions);

int backupStagingArea();
int restoreStageingBackup();
int fetchStagingArea();
int fetchHEAD();

/**
 * @brief Obtain the path of the repository containing a given working directory.
 *
 * The obtainRepository function takes a working directory path (workDir) as input
 * and searches for the repository path containing it. It iteratively traverses the
 * directory tree upward, looking for a directory with a name matching the program name
 * (e.g., ".neogit" ). Once found, it returns the path to that repository.
 *
 * @param workDir The working directory path for which the repository path is sought.
 *
 * @return A dynamically allocated string representing the repository path. If a matching
 * repository is not found, or if memory allocation fails, NULL is returned. The caller
 * is responsible for freeing the memory allocated for the result.
 */
int obtainRepository(constString workDir);

/**
 * @brief Set a configuration key-value pair in the specified configuration file.
 *
 * The setConfig function sets or updates the value associated with the given key in
 * the configuration file. The configuration file is determined based on the 'global' parameter,
 * and the key-value pair is added or modified according to the provided information.
 *
 * @param key The configuration key to set or update.
 * @param value The new value for the specified key.
 * @param now The timestamp associated with the key-value pair.
 * @param global If true, the global configuration file is used; otherwise, the
 *               repository-specific configuration file is used.
 *
 * @return ERR_NOERR if the operation is successful; otherwise, an error code is returned.
 *         Possible error codes include ERR_FILE_ERROR (unable to open or access the configuration file)
 *         and other error codes related to file manipulation.
 */
int setConfig(constString key, constString value, time_t now, bool global);

/**
 * @brief Retrieve a configuration value based on the specified key.
 *
 * The getConfig function obtains a configuration value for the given key. It first checks
 * the local (repository-specific) configuration file. If the key is not found or has an older timestamp,
 * it falls back to the global configuration file. The retrieved value is returned.
 *
 * Note: If the repository is not initialized, the global configuration is used regardless of timestamps.
 *
 * @param key The configuration key to retrieve its value.
 *
 * @return The retrieved configuration value. If the key is not found or an error occurs, NULL is returned.
 *         The returned string is dynamically allocated and should be freed by the caller when no longer needed.
 */
String getConfig(constString key);

/**
 * @brief Remove a configuration entry with the specified key from the configuration file.
 *
 * The removeConfig function removes the configuration entry with the given key from the configuration file.
 * It supports both global and local configuration files. The function opens the configuration file, searches
 * for the specified key, removes the corresponding line, and then closes the file.
 *
 * @param key The key of the configuration entry to be removed.
 * @param global A boolean indicating whether to operate on the global configuration file.
 *
 * @return An integer representing the error code. Returns ERR_NOERR if successful, otherwise an error code.
 */
int removeConfig(constString key, bool global);

/**
 * @brief Retrieve all alias keys from both global and local configurations.
 *
 * The getAliases function retrieves all alias keys from both the global and local configuration files.
 * It avoids duplications and returns the total number of unique alias keys found.
 *
 * @param keysDest An array provided by the caller to store the retrieved alias keys.
 *                 The array should be allocated with enough space to accommodate all unique alias keys found.
 *                 The function populates this array with alias keys, and the caller is responsible for freeing the memory.
 *
 * @return The total number of unique alias keys found. If an error occurs or no alias keys are found, 0 is returned.
 */
int getAliases(String *keysDest);
bool isGitIgnore(FileEntry *entry);
int removeFromStage(constString filePath);
int addToStage(constString filePath);
int trackFile(constString filepath);

FileEntry getRepoFileEntry(constString path);
int lsWithHead(FileEntry **buf, constString path);
int lsChangedFiles(FileEntry **buf, constString dest);

Commit *createCommit(StagedFileArray *filesToCommit, String fromWhere, String username, String email, String message);
Commit *getCommit(uint64_t hash);
void freeCommitStruct(Commit *object);
uint64_t getBrachHeadPrev(constString branchName, uint order);
uint64_t getBranchHead(constString branchName);
int setBranchHead(constString branchName, uint64_t commitHash);
int listBranches(String *namesDest, uint64_t *hashesDest);

StagedFile *getStagedFile(constString path);
bool isTrackedFile(constString path);
int applyToWorkingDir(StagedFileArray head);
bool isWorkingTreeModified();

ChangeStatus getChangesFromHEAD(constString path);
ChangeStatus getChangesFromStaging(constString path);

#endif