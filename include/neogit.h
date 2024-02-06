/*******************************
 *         neogit.h            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#ifndef __NEOGIT_H__
#define __NEOGIT_H__

#include "common.h"
#include "string_funcs.h"
#include "file_funcs.h"

#define PROGRAM_NAME "neogit"

// Struct representing each command of program (command, min/max argc, usage help, ..)
typedef struct _command_t
{
	constString key;						   /**< The key representing the command. */
	uint minArgc;							   /**< Minimum required number of arguments for the command. */
	uint maxArgc;							   /**< Maximum allowed number of arguments for the command. (-1 for no limit) */
	int (*function)(int, constString[], bool); /**< Function pointer to the command handler function. */
	constString usageHelp;					   /**< Usage help string for the command. */
} Command;

/**
 * @brief Process and execute a command or check the command syntax. (main.c)
 *
 * The process_command function takes the command-line arguments and performs various checks.
 * It iterates through the available commands, checks if the provided command is valid, and executes
 * the corresponding function or displays an error message.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @param performActions A boolean indicating whether to perform actions or only check the syntax.
 *
 * @return An integer representing the error code. Returns ERR_NOERR if successful, otherwise an error code.
 */
int process_command(int argc, constString argv[], bool performActions);

/////////////////////////// STRUCTS OF GIT REPOSITORY ////////////////////////

// The GitObject struct represents a file in the staging area or commit, including the file details and a 10-digit hash string.
typedef struct _git_object_t
{
	FileEntry file;	  /**< Details of the object file. */
	char hashStr[11]; /**< 10-digit hash string. */
} GitObject;
// The GitObjectArray struct holds an array of GitObject elements along with the length of the array.
typedef struct _git_object_array_t
{
	GitObject *arr; /**< Array of git objects. */
	uint len;		/**< Length of the array. */
} GitObjectArray;

// Struct representing the HEAD of the repository.
typedef struct _head_t
{
	uint64_t hash;			  /**< Hash of the HEAD state. */
	String branch;			  /**< Current branch. */
	GitObjectArray headFiles; /**< Array of files in the HEAD state. */
} HEAD;

// Struct representing a commit in the repository.
typedef struct _commit_t
{
	uint64_t hash;				  /**< Hash of the commit. */
	String username;			  /**< Username of the committer. */
	String useremail;			  /**< Email of the committer. */
	time_t time;				  /**< Commit time. */
	String message;				  /**< Commit message. */
	String branch;				  /**< Branch associated with the commit. */
	GitObjectArray commitedFiles; /**< Array of files committed in this commit. */
	GitObjectArray headFiles;	  /**< Array of files in the HEAD state after this commit. */
	uint64_t prev;				  /**< Hash of the previous commit. */
	uint64_t mergedCommit;		  /**< Hash of the merged commit (if applicable). */
} Commit;

// The Repository struct holds information about a repository (path, staging area, head)
typedef struct _repository_t
{
	String absPath;				/**< Absolute path of the repository. */
	GitObjectArray stagingArea; /**< Array of files in the staging area. */
	HEAD head;					/**< HEAD state of the repository. */
	bool deatachedHead;			/**< Flag indicating whether the HEAD is detached. */
} Repository;

// The Tag struct
typedef struct _tag_t
{
	String tagname;
	String message;
	uint64_t commitHash;
	time_t tagTime;
	String authorName;
	String authorEmail;
} Tag;

// Enumeration representing the change status of a file.
typedef enum _change_status_t
{
	NOT_CHANGED = 0, /**< File not changed. */
	MODIFIED,		 /**< File modified. */
	ADDED,			 /**< File added. */
	DELETED,		 /**< File deleted. */
	PERM_CHANGED	 /**< File permission changed. */
} ChangeStatus;

// Enumeration representing the change status of a file
typedef enum _conflicting_status_t
{
	SAME_BINARY = 0,   /**< Two files are completely same in bytes */
	SAME_TEXT,		   /**< Two files have same text content (via diff) */
	NEW_FILE,		   /**< A new file added and not found in base */
	REMOVED_IN_BASE,   /**< Two files are found in both databases, but the base one is marked as deleted */
	REMOVED_IN_TARGET, /**< Two files are found in both databases, but the target one is marked as deleted */
	CONFLICT		   /**< Two files are present and have conflicts */
} ConflictingStatus;

/////////////////////////// GENERAL FUNCTIONS + CONFIG and ALIAS ////////////////////////

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

///////////////////// FUNCTIONS RELATED TO STAGING AREA / ADD / Status ////////////////////////

/**
 * @brief Checks if the given entry is a git ignored object.
 * @param entry The file entry to be checked.
 * @return Returns true if the entry is a git ignored object.
 */
bool isGitIgnore(FileEntry *entry);

/**
 * @brief Tracks a file by adding it to the list of tracked files.
 *
 * This function checks if the file is already tracked. If not, it adds the file path
 * to the manifest file that keeps track of all the files being monitored.
 *
 * @param filepath <<Must be relative to repo path>>.
 * @return Returns ERR_NOERR if the operation is successful, otherwise an error code.
 */
int trackFile(constString filepath);

/**
 * @brief Checks if a file is tracked.
 *
 * This function reads the manifest file containing the list of tracked files and
 * checks if the specified file path is present in the list.
 *
 * @param path <<Must be relative to repo path>>.
 * @return Returns true if the file is tracked, otherwise false.
 */
bool isTrackedFile(constString path);

/**
 * @brief Generates a unique identifier.
 *
 * This function generates a unique identifier based on the current time and random values.
 *
 * @param hexdigits The number of hexadecimal digits in the generated identifier.
 * @return Returns the generated unique identifier.
 */
ullong generateUniqueId(int hexdigits);

/**
 * @brief Adds a file to the staging area.
 *
 * This function adds the specified file to the staging area, updating its information in the staging area and info file.
 *
 * @param filePath <<Must be relative to repo path>>.
 * @return Returns an error code. ERR_NOERR on success, other codes on failure.
 */
int addToStage(constString filePath);

/**
 * @brief Removes a file from the staging area.
 *
 * This function removes the specified file from the staging area, updating its information in the staging area and info file.
 *
 * @param filePath <<Must be relative to repo path>>.
 * @return Returns an error code. ERR_NOERR on success, ERR_NOT_EXIST if the file is not in the staging area, other codes on failure.
 */
int removeFromStage(constString filePath);

/**
 * @brief Creates backups of the staging area by moving existing backups and copying the current staging area.
 *
 * This function creates backups of the staging area by moving existing backups to newer versions (old0 to old1, old1 to old2, ...),
 * and then copying the current staging area to old0.
 *
 * @return Returns 0 if successful, or an error code if there was an issue during the backup process.
 */
int backupStagingArea();

/**
 * @brief Restores the staging area from the backup (old0) by moving backed up files and removing the backup directory.
 *
 * This function restores the staging area by moving files from the backup (old0) to the staging area and
 * then removing the backup directory.
 *
 * @return Returns 0 if successful, or an error code if there was an issue during the restoration process.
 */
int restoreStageingBackup();

/**
 * @brief Gets the StagedFile corresponding to the provided file path.
 *
 * This function searches for the GitObject in the GitObjectArray of the current repository based on the provided file path.
 *
 * @param path <<Must be relative to repo path>>.
 * @return Returns a pointer to the GitObject if found, or NULL if not found.
 */
GitObject *getStagedFile(constString path);

/**
 * @brief Fetches the information of files in the staging area from the info file.
 *
 * This function reads the information of files in the staging area from the info file and updates the GitObjectArray in the curRepository.
 *
 * @return Returns an error code. ERR_NOERR on success, ERR_FILE_ERROR on file-related errors.
 */
int fetchStagingArea();

/**
 * @brief Get the change status of a file relative to the staging area and the HEAD commit.
 *
 * This function determines the change status of a file by comparing its state in the working directory,
 * staging area, and the HEAD commit.
 * @note - Example : if a file is commited, and not exist in staging area, compare the working tree version with HEAD version
 *  - but if staging version is persent, determination compare with staged version.
 *
 * @param path <<Must be relative to repo path>>.
 * @return Returns the change status of the file (NOT_CHANGED, MODIFIED, ADDED, DELETED, or PERM_CHANGED).
 */
ChangeStatus getChangesFromStaging(constString path);

/**
 * @brief Processes the directory tree starting from the given root.
 *
 * This function recursively traverses the directory tree starting from the provided root.
 * It applies the specified list function to obtain child entries and performs actions based on the provided options.
 *
 * @param root The root of the directory tree to start processing. <<its path must be absolute or relative to cwd>>
 * @param curDepth The current depth of recursion.
 * @param listFunction The function to obtain child entries of a directory. <<must accept abs path and give abs path>>
 * @param print_tree Whether to print the directory tree structure.
 * @param callbackFunction The callback function to be applied to each processed entry. the given path to this function, is <<relative to repo>>.
 * @return Returns the number of processed entries.
 */
int processTree(FileEntry *root, uint curDepth, int (*listFunction)(FileEntry **, constString), bool print_tree, void (*callbackFunction)(FileEntry *));

/**
 * @brief List files in the specified directory, including files from the HEAD commit that are not in the working directory.
 *
 * This function lists files in the specified directory, and includes files from the HEAD commit that are not present
 * in the working directory. <<The paths in the output buffer are absolute>>
 *
 * @param buf Pointer to the buffer where the list of files will be stored.
 * @param path The relative-to-cwd or absolute path of the directory to list.
 * @return Returns the number of entries in the list on success, -2 if the specified path is a files,
 * or -1 if an error occurs during listing.
 */
int lsWithHead(FileEntry **buf, constString path);

/**
 * @brief List files with changes in the specified destination directory.
 *
 * This function lists files with changes in the specified destination directory. It includes both files that have
 * changes in the working directory and files that have changes staged in the HEAD commit. <<The paths in the output
 * buffer are absolute>>.
 *
 * @param buf Pointer to the buffer where the list of files with changes will be stored.
 * @param dest The relative-to-cwd or absolute path of the destination directory to list.
 * @return Returns the number of entries in the list on success, or an error code if an error occurs during listing.
 */
int lsChangedFiles(FileEntry **buf, constString dest);
#define _LIST_FILES_CHANGED_FROM_HEAD ({extern bool _ls_head_changed_files; _ls_head_changed_files=true; })
#define _LIST_FILES_CHANGED_FROM_STAGE ({extern bool _ls_head_changed_files; _ls_head_changed_files=false; })

///////////////////// FUNCTIONS RELATED TO COMMITS/BRANCH/CHECKOUT/... ////////////////////////

/**
 * @brief Copy a GitObjectArray structure.
 *
 * This function copies a GitObjectArray structure, including its fields and arrays to dest
 *
 * @param dest destination GitObjectArray (The caller is responsible for freeing the allocated memories)
 * @param src source GitObjectArray
 */
void copyGitObjectArray(GitObjectArray *dest, GitObjectArray *src);

/**
 * @brief Free the memory allocated for a GitObjectArray structure.
 *
 * This function frees the memory allocated for a GitObjectArray structure, including its fields and arrays.
 *
 * @param object Pointer to the GitObjectArray structure to be freed.
 */
void freeGitObjectArray(GitObjectArray *array);

/**
 * @brief Create a new commit with the specified changes.
 *
 * This function creates a new commit with the specified changes and updates the repository's commit history.
 *
 * @param filesToCommit A pointer to the GitObjectArray containing the staged files to be committed.
 * @param username The username of the commit author.
 * @param email The email address of the commit author.
 * @param message The commit message.
 * @param mergedHash must be zero for normal commits. Provided in Megring Action and merging commits!
 * @return Returns a pointer to the newly created commit on success, or NULL if the commit creation fails.
 */
Commit *createCommit(GitObjectArray *filesToCommit, constString username, constString email, constString message, uint64_t mergedHash);

/**
 * @brief Retrieve a commit by its hash.
 *
 * This function retrieves a commit from the repository by its hash.
 *
 * @param hash The hash of the commit to retrieve.
 * @return Returns a pointer to the retrieved commit on success, or NULL if the commit is not found or an error occurs.
 */
Commit *getCommit(uint64_t hash);

/**
 * @brief Free the memory allocated for a Commit structure.
 *
 * This function frees the memory allocated for a Commit structure, including its fields and arrays.
 *
 * @param object Pointer to the Commit structure to be freed.
 */
void freeCommitStruct(Commit *object);

/**
 * @brief Lists the branches in the repository, along with their hashes.
 *
 * @param namesDest     Pointer to the destination array to store branch names.
 * @param hashesDest    Pointer to the destination array to store branch hashes.
 * @return              The number of branches found or -1 on failure.
 */
int listBranches(String *namesDest, uint64_t *hashesDest);

/**
 * @brief Sets the head of the specified branch to the given commit hash.
 *
 * This function updates the head of the specified branch with the provided
 * commit hash in the branches file. (if branch not exist, creates it.)
 *
 * @param branchName    The name of the branch to set the head for.
 * @param commitHash    The commit hash to set as the head for the branch.
 * @return              Returns ERR_NOERR on success, ERR_FILE_ERROR if there
 *                      is an issue with the branches file, or the result of
 *                      searchLine/replaceLine if an error occurs during file
 *                      processing.
 */
int setBranchHead(constString branchName, uint64_t commitHash);

/**
 * @brief Retrieves the commit hash associated with the head of the specified branch.
 *
 * This function looks up the commit hash associated with the head of the specified
 * branch in the branches file.
 *
 * @param branchName    The name of the branch to retrieve the head commit hash for.
 * @return              Returns the commit hash if successful, or 0x1FFFFFF if the
 *                      branch does not exist. If there is an issue with the branches
 *                      file, it returns ERR_FILE_ERROR.
 */
uint64_t getBranchHead(constString branchName);

/**
 * @brief Retrieves the commit hash at a specified order before the head of the branch.
 *
 * This function retrieves the commit hash at a specified order before the head of
 * the specified branch. It iterates through the previous commits based on the order
 * parameter.
 *
 * @param branchName    The name of the branch to retrieve the commit hash for.
 * @param order         The order of the commit to retrieve relative to the head.
 * @return              Returns the commit hash if successful, or 0xFFFFFF if there
 *                      is an issue retrieving the commit information.
 */
uint64_t getBrachHeadPrev(constString branchName, uint order);

/**
 * @brief Retrieves the GitObject corresponding to a file in the given GitObjectArray
 *
 * This function searches for the GitObject associated with the specified file path
 * within the given GitObjectArray. The input path <<must be relative to the repository.>>
 *
 * @param path <<must be relative to the repository.>>
 * @param head the GitObjectArray (which search within)
 * @return Returns a pointer to the GitObject if found, otherwise returns NULL.
 */
GitObject *getHEADFile(constString path, GitObjectArray head);

/**
 * @brief Fetches information related to the HEAD of the repository.
 *
 * This function retrieves information about the HEAD of the repository,
 * including the branch name and commit hash. If the HEAD is detached, it
 * sets the 'detachedHead' flag to true. (initialize the curRepository->head)
 *
 * @return Returns ERR_NOERR if the operation is successful, otherwise an error code.
 */
int fetchHEAD();

/**
 * @brief Determines the status of a file relative to the given GitObjectArray files.
 *
 * This function compares the specified file path relative to the repository with
 * its corresponding GitObject in the GitObjectArray to determine its change status.
 *
 * @param path <<must be relative to the repository.>>
 * @param head the GitObjectArray
 * @return Returns the ChangeStatus indicating the file's status relative to the given head array. (NULL if not found)
 */
ChangeStatus getChangesFromHEAD(constString path, GitObjectArray head);

/**
 * @brief Checks if the working tree is modified compared to the HEAD commit.
 *
 * This function iterates over the tracked files in the repository and checks
 * if there are any changes in the working tree compared to the HEAD commit.
 * @note This function just check the tracked files! not untracked ones!
 * @return Returns true if the working tree is modified; otherwise, returns false.
 */
bool isWorkingTreeModified();

/**
 * @brief Apply changes from the gitObjectArray to the working directory.
 *
 * This function iterates over the tracked files in the repository and applies
 * changes from the given gitObjectArray to the working directory based on the status
 * of each file.
 *
 * @warning Dangerous function! always pay attention and note down what you are doing.
 *
 * @param head The GitObjectArray that will be applied to working tree
 * @return Returns ERR_NOERR on success; otherwise, returns an error code.
 */
int applyToWorkingDir(GitObjectArray head);

///////////////////// FUNCTIONS RELATED TO TAG/DIFF/MERGE ////////////////////////

/**
 * @brief Retrieves the branch to which the specified branch was merged, if any.
 *
 * This function checks if the given branch has been merged and returns the name of
 * the branch to which it was merged. The returned string is dynamically allocated
 * and should be freed after use.
 *
 * @param branch The name of the branch to check for a merged destination.
 * @return       The name of the merged destination branch or NULL if not merged.
 *               The returned string should be freed by the caller after use.
 */
String getMergeDestination(constString branch);

/**
 * @brief Prints the difference between two files in a side-by-side format.
 *
 * This function displays the differences between two files in a side-by-side format,
 * showing added and removed lines for each corresponding pair of lines in the files.
 * It prints the line numbers and the content of the added and removed lines.
 *
 * @param diff           The diff structure containing information about the changes.
 * @param f1PathToShow   The path of the first file to display in the diff.
 * @param f2PathToShow   The path of the second file to display in the diff.
 */
void printDiff(Diff *diff, constString f1PathToShow, constString f2PathToShow);

/**
 * @brief Determines the conflicting status of a file during a merge.
 *
 * This function determines the conflicting status of a file during a merge operation.
 * It compares the target object with the base object and checks for conflicts.
 *
 * @param targetObj  The target Git object.
 * @param base       An array of Git objects from the base branch.
 * @param diffDest   Pointer to a Diff structure to store the difference in case of conflict.
 * 
 * @note - In case of CONFLICT, if diffDest provided, it will  be filled with diff information; and should be freeDiffStruct after use.
 * @return           The ConflictingStatus indicating the conflicting status of the file.
 */
ConflictingStatus getConflictingStatus(GitObject *targetObj, GitObjectArray base, Diff* diffDest);

/**
 * @brief Lists tags associated with a commit or all tags in the repository.
 *
 * This function retrieves information about tags from the tag manifest file.
 * If a commit hash is provided, it filters tags associated with that specific commit.
 * The resulting tags are sorted alphabetically by name.
 *
 * @param destBuf      Pointer to the destination buffer to store the tags.
 * @param commitHash   The commit hash to filter tags (use 0 for all tags).
 * @return             The number of tags found and stored in the destination buffer.
 */
int listTags(Tag **buf, uint64_t commitHash);

/**
 * @brief Retrieves information about a specific tag.
 *
 * This function looks for the specified tag in the tag manifest file and returns
 * information about that tag if found.
 *
 * @param tag_name   The name of the tag to retrieve.
 * @return           A pointer to the Tag structure containing tag information,
 *                   or NULL if the tag is not found.
 */
Tag *getTag(constString tag_name);

/**
 * @brief Sets or updates a tag in the tag manifest file.
 *
 * This function adds a new tag entry or updates an existing tag entry in the tag manifest file.
 *
 * @param tag_name      The name of the tag to set or update.
 * @param message       The message associated with the tag.
 * @param commitHash    The hash of the commit the tag points to.
 * @param author_name   The name of the author of the tag.
 * @param author_email  The email of the author of the tag.
 * @param time          The timestamp of the tag.
 * @return              Returns ERR_NOERR on success, or an error code if an error occurs.
 */
int setTag(constString tag_name, constString message, uint64_t commitHash, constString author_name, constString author_email, time_t time);

/**
 * @brief Frees the memory allocated for an array of Tag structures.
 *
 * This function deallocates the memory for each field of each Tag structure in the array,
 * and then frees the memory of the array itself.
 *
 * @param array   The array of Tag structures to be freed.
 * @param length  The number of elements in the array.
 */
void freeTagStruct(Tag *array, uint length);

#endif
