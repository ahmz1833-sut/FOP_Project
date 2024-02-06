/*******************************
 *         phase2.c            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "phase2.h"
#include "phase1.h" // Using command_checkout

extern String curWorkingDir;	  // Declared in neogit.c
extern Repository *curRepository; // Declared in neogit.c

int command_revert(int argc, constString argv[], bool performActions)
{
	uint sIdx = checkAnyArgument("-n");
	bool noCommit = false;
	uint64_t targetHash = 0;
	constString targetStr = NULL, showingTarget = "HEAD", message = NULL, name = NULL, email = NULL;

	if (sIdx) // Check for -n switch
	{
		if (argc != 3)
			return ERR_ARGS_MISSING;
		targetStr = (sIdx == 1) ? argv[2] : argv[1];
		noCommit = true;
	}
	else
	{
		uint mIdx = checkAnyArgument("-m"); // check for switch -m (change message)
		if (mIdx == 2 || mIdx == 1)
		{
			if (argc != 4)
				return ERR_ARGS_MISSING;

			targetStr = (mIdx == 1) ? argv[3] : argv[1];
			message = argv[mIdx + 1];
		}
		else if (mIdx && argc != 2)
			return ERR_ARGS_MISSING;
		else
			targetStr = argv[1];
	}

	if (!performActions) // Syntax Check OK!
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	if (isMatch(targetStr, "HEAD"))
		targetHash = curRepository->head.hash;
	else if (isMatch(targetStr, "HEAD-*"))
	{
		uint order = 0;
		if ((sscanf(targetStr, "HEAD-%u", &order) != 1) || !order)
			return ERR_ARGS_MISSING;
		targetHash = getBrachHeadPrev(curRepository->head.branch, order) & 0xFFFFFF;
		if (targetHash == 0xFFFFFF)
		{
			printError("Unable to revert to HEAD-%u : Parent does not exist!", order);
			return ERR_NOT_EXIST;
		}
		showingTarget = targetStr;
	}
	else if (!((sscanf(showingTarget = targetStr, "%lx", &targetHash) == 1) && targetHash))
		return ERR_ARGS_MISSING;

	// check if hash found in prev commits and there is no mereging action in history
	uint64_t tmpHash = curRepository->head.hash;
	while (tmpHash != targetHash)
	{
		Commit *tmpCommit = getCommit(tmpHash);
		if (tmpCommit == NULL)
		{
			printWarning("ERROR in Revert : The requested target was not found in history of HEAD.\n");
			return ERR_GENERAL;
		}

		if (tmpCommit->mergedCommit != 0)
		{
			printWarning("ERROR in Revert : Note that there is a merging action in the history of current HEAD.\n You Cannot revert this merging action!\n");
			freeCommitStruct(tmpCommit);
			return ERR_GENERAL;
		}

		tmpHash = tmpCommit->prev;
		freeCommitStruct(tmpCommit);
	}

	Commit *c = getCommit(targetHash);
	if (c != NULL) // commit hash found!
	{
		if (!noCommit) // check username and email if we need create a new commit
		{
			// Check the user.name and user.email
			name = getConfig("user.name");
			email = getConfig("user.email");
			if (!name || !email)
			{
				printError("You haven't been configured the user information!");
				printError("Please submit your information and configs for NeoGIT!\n");
				printWarning("You must first set them by Command(s) below:");
				if (!name)
					printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.name <yourname>\"" _RST);
				else
					free((void *)name);
				if (!email)
					printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.email <youremail@example.com>\"" _RST);
				else
					free((void *)email);
				return ERR_CONFIG_NOTFOUND;
			}
			if (!message)
				message = c->message;
		}

		if (applyToWorkingDir(c->headFiles) != ERR_NOERR) // Revert to commit
		{
			freeCommitStruct(c);
			printError("Failed to reverting working tree to " _BOLD "'%s'" _UNBOLD "!", showingTarget);
		}

		printf("The working tree has been reverted to " _CYANB "'%s'" _RST " successfully!\n\n", showingTarget);

		if (!noCommit) // if we must create a new commit
		{
			Commit *res = createCommit(&(c->headFiles), name, email, message, 0);
			free((void *)name);
			free((void *)email);

			if (res)
			{
				printf("Successfully performed the commit: " _CYANB "'%s'\n" _RST, message);
				char datetime[DATETIME_STR_MAX];
				strftime(datetime, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&res->time));
				printf("Date and Time : " _BOLD "%s\n" _RST, datetime);
				printf("on branch " _CYANB "'%s'" _RST " - commit hash " _CYANB "'%06lx'\n" _RST, res->branch, res->hash);
				printf(_DIM "[" _BOLD "%u" _UNBOLD _DIM " file(s) commited]\n" _UNBOLD _RST "\n", res->commitedFiles.len);
			}
			else
			{
				printError("Error in perform commit!\n");
				return ERR_UNKNOWN;
			}
		}

		freeCommitStruct(c);
	}
	else
	{
		printError("Error! commit not found!");
		return ERR_NOT_EXIST;
	}
}

int command_tag(int argc, constString argv[], bool performActions)
{
	if (argc == 1) // show list of tags
	{
		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		Tag *buf = NULL;
		int tags_count = listTags(&buf, 0);

		if (!tags_count)
			printWarning("There is no tags in your repository!\n");

		for (int i = 0; i < tags_count; i++)
		{
			constString arg[] = {"", "show", buf[i].tagname};
			command_tag(3, arg, true); // show tag
		}

		freeTagStruct(buf, tags_count);
		return ERR_NOERR;
	}
	else if (argc == 3 && isMatch(argv[1], "show")) // show a tag information
	{
		if (!performActions)
			return ERR_NOERR;

		Tag *tag = getTag(argv[2]);
		if (!tag)
		{
			printError("The Tag with name you entered does not found!\n");
			return ERR_NOT_EXIST;
		}

		printf(_REDB "*" _RST " Tag " _MAGNTA _BOLD "'%s'" _RST "\n", tag->tagname);
		printf("  Commit " _YELB "'%06lx'" _RST "\n", tag->commitHash);
		char datetime[DATETIME_STR_MAX];
		strftime(datetime, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&tag->tagTime));
		printf("  Date and Time : " _BOLD "%s\n" _RST, datetime);
		printf("  Author: " _CYANB "%s <%s>" _RST "\n", tag->authorName, tag->authorEmail);
		if (strcmp(tag->message, "(null)") != 0)
			printf("  Message: " _CYANB "'%s'\n" _RST, tag->message);
		printf("\n"); 
		freeTagStruct(tag, 1);
		return ERR_NOERR;
	}
	else if (checkAnyArgument("-a")) // add a tag
	{
		uint aIdx = checkAnyArgument("-a");
		uint mIdx = checkAnyArgument("-m");
		uint cIdx = checkAnyArgument("-c");
		uint fIdx = checkAnyArgument("-f");

		if (argc < aIdx + 1 || argc < mIdx + 1 || argc < cIdx + 1)
			return ERR_ARGS_MISSING;

		argc -= 3; // tag -a <tag>
		if (mIdx)
			argc -= 2;
		if (cIdx)
			argc -= 2;
		if (fIdx)
			argc--;

		if (argc != 0)
			return ERR_ARGS_MISSING;

		char tagName[100], message[STR_LINE_MAX];
		uint invalid = strValidate(tagName, argv[aIdx + 1], VALID_CHARS);
		if (isEmpty(tagName))
			return ERR_ARGS_MISSING;

		if (mIdx)
			strValidate(message, argv[mIdx + 1], VALID_CHARS);
		else
			strcpy(message, "(null)");

		uint64_t commitHash = curRepository->head.hash; // default : HEAD
		if (cIdx && (sscanf(argv[cIdx + 1], "%lx", &commitHash) != 1))
			return ERR_ARGS_MISSING;

		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		Commit *commit = getCommit(commitHash);
		if (!commit)
		{
			printError("Error! Could not find the specified commit.\n");
			return ERR_NOT_EXIST;
		}
		freeCommitStruct(commit);

		Tag *oldTag = getTag(tagName);
		if (oldTag) // if a tag with this tag-name already exists
		{
			freeTagStruct(oldTag, 1);
			if (!fIdx) // if switch -f is  not provided
			{
				printWarning("A tag with this tag name already exist. You can show it using " _BOLD "'neogit tag show %s'" _UNBOLD " command.", tagName);
				printWarning("Use '-f' to force replacing the new tag.\n");
				return ERR_ALREADY_EXIST;
			}
		}

		time_t tagTime = time(NULL);

		// Check the user.name and user.email
		String name = getConfig("user.name");
		String email = getConfig("user.email");
		if (!name || !email)
		{
			printError("You haven't been configured the user information!");
			printError("Please submit your information and configs for NeoGIT!\n");
			printWarning("You must first set them by Command(s) below:");
			if (!name)
				printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.name <yourname>\"" _RST);
			else
				free(name);
			if (!email)
				printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.email <youremail@example.com>\"" _RST);
			else
				free(email);
			return ERR_CONFIG_NOTFOUND;
		}

		int result = setTag(tagName, message, commitHash, name, email, tagTime);
		if (result == ERR_NOERR)
			printf("The tag named " _CYANB "\"%s\"" _RST " has been %s successfully!\n\n", tagName, oldTag ? "updated" : "created");
		else
			printError("Error is creating tag \"%s\".\n", tagName);
		free(name);
		free(email);

		constString arg[] = {"", "show", tagName};
		command_tag(3, arg, true); // show tag
		return result;
	}
	return ERR_ARGS_MISSING;
}

int command_grep(int argc, constString argv[], bool performActions)
{
	uint fIdx = checkAnyArgument("-f"); // file
	uint pIdx = checkAnyArgument("-p"); // pattern
	uint cIdx = checkAnyArgument("-c"); // commit (optional)
	uint nIdx = checkAnyArgument("-n"); // show line number flag

	if ((cIdx && nIdx && argc != 8) ||
		(cIdx && !nIdx && argc != 7) ||
		(!cIdx && nIdx && argc != 6) ||
		(!cIdx && !nIdx && argc != 5))
		return ERR_ARGS_MISSING;

	if (argc < fIdx + 1 || argc < pIdx + 1 || argc < cIdx + 1)
		return ERR_ARGS_MISSING;

	constString filename = argv[fIdx + 1];
	constString pattern = argv[pIdx + 1];
	uint64_t commitHash = 0; // default : search in working tree
	if (cIdx && (sscanf(argv[cIdx + 1], "%lx", &commitHash) != 1))
		return ERR_ARGS_MISSING;

	// syntax OK
	if (!performActions)
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	char absPath[PATH_MAX], relrepoPath[PATH_MAX];
	String relative_to_repo = normalizePath(filename, curRepository->absPath); // obtain file path relative to repo
	if (relative_to_repo)
		strcpy(relrepoPath, relative_to_repo);
	else
	{
		printError("File is not belongs your repository!!\n");
		return ERR_NOT_EXIST;
	}
	free(relative_to_repo);

	if (commitHash) // if commit specified
	{
		Commit *c = getCommit(commitHash);
		if (c == NULL)
		{
			printError("Commit does not exist!\n");
			return ERR_NOT_EXIST;
		}
		GitObject *obj = getHEADFile(relrepoPath, c->headFiles);
		if (!obj || obj->file.isDeleted)
		{
			printError("The file is not available at the specified commit.\n");
			return ERR_NOT_EXIST;
		}
		strcat_s(absPath, curRepository->absPath, "/.neogit/objects/", obj->hashStr); // path to related object
		freeCommitStruct(c);
	}
	else
	{
		strcat_s(absPath, curRepository->absPath, "/", relrepoPath); // path to real file in the working tree
		if (access(absPath, F_OK) != 0)
		{
			printError("The file is not available in your working tree.\n");
			return ERR_NOT_EXIST;
		}
	}

	printf("\n");
	tryWithFile(file, absPath, ({ return ERR_FILE_ERROR; }), __retTry)
	{
		uint lineIndex = 0;
		char buf[STR_LINE_MAX];
		uint totalOccurence = 0;
		while (++lineIndex && fgets(buf, STR_LINE_MAX, file))
		{
			strtrim(buf);
			char dest[STR_LINE_MAX];
			if (strReplace(dest, buf, pattern, boldAndUnderlineText)) // if matchings are found
			{
				totalOccurence += strReplace(NULL, buf, pattern, NULL);
				printf(_DIM "Line %d:\t" _RST "%s\n", lineIndex, dest);
			}
		}

		if (totalOccurence)
			printf(_GRNB "\nTotal occurences: " _CYAN "%u" _RST "\n\n", totalOccurence);
		else
			printf(_YELB "No Occurence!\n\n" _RST);
	}
	return ERR_NOERR;
}

int command_diff(int argc, constString argv[], bool performActions)
{
	if (checkArgument(1, "-c")) // diff commits!
	{
		if (argc != 4)
			return ERR_ARGS_MISSING;

		// argv[2]  argv[3]
		constString commit1HashStr = argv[2];
		uint64_t hash1 = 0;
		constString commit2HashStr = argv[3];
		uint64_t hash2 = 0;

		if (!((sscanf(commit1HashStr, "%lx", &hash1) == 1) && hash1)) // Try to scan a hexadecimal hash.
			return ERR_ARGS_MISSING;

		if (!((sscanf(commit2HashStr, "%lx", &hash2) == 1) && hash2)) // Try to  scan another one...
			return ERR_ARGS_MISSING;

		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		Commit *c1 = getCommit(hash1);
		Commit *c2 = getCommit(hash2);
		if (!c1 || !c2)
		{
			printError("Commit hash not found!!\n");
			freeCommitStruct(c1);
			freeCommitStruct(c2);
			return ERR_NOT_EXIST;
		}

		// Print the header for this comparison.
		printf("Comparing Commits " _CYANB "%s" _RST "  .....  " _YELB "%s\n" _RST, commit1HashStr, commit2HashStr);
		printf("  " _CYANB "\"%s\"" _RST " ..... " _YELB "\"%s\"\n" _RST, c1->message, c2->message);
		char datetime1[DATETIME_STR_MAX], datetime2[DATETIME_STR_MAX];
		strftime(datetime1, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&c1->time));
		strftime(datetime2, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&c2->time));
		printf("  " _CYANB "%s" _RST " ..... " _YELB "%s\n" _RST, datetime1, datetime2);
		printf("  " _CYANB "%s <%s>" _RST " ..... " _YELB "%s <%s>\n" _RST, c1->username, c1->useremail, c2->username, c2->useremail);
		printf("\n");

		bool different = false;

		// iterate over
		for (int i = 0; i < c2->headFiles.len; ++i)
		{
			Diff diff;
			ConflictingStatus state = getConflictingStatus(&(c2->headFiles.arr[i]), c1->headFiles, &diff);
			String fpath = c2->headFiles.arr[i].file.path;

			switch (state)
			{
			case SAME_BINARY:
				continue;
			case SAME_TEXT:
				printf("\nFiles with path " _BOLD "%s" _UNBOLD " have binary differences between two commits; but there is no text difference.\n", fpath);
				break;
			case NEW_FILE:		  // files in c2 that are not present in c1
			case REMOVED_IN_BASE: // files in c2 that are marked deleted in c1
				printf("\nFile " _YELB "%s" _RST " is not found in commit " _CYANB "'%s'" _RST ", but it present in commit " _YELB "'%s'" _RST ".\n", fpath, commit1HashStr, commit2HashStr);
				break;
			case REMOVED_IN_TARGET: // REMOVED_IN_TARGET (TARGET means c2 here)
				continue;			// handled later (in next loop)
			case CONFLICT:
				char str1[PATH_MAX], str2[PATH_MAX];
				sprintf(str1, "<%s>/%s", commit1HashStr, fpath);
				sprintf(str2, "<%s>/%s", commit2HashStr, fpath);
				printf("\n");
				printDiff(&diff, str1, str2);
				freeDiffStruct(&diff);
				break;
			}

			different = true;
		}

		// reverse  iteration to handle removed files in commit2 (or new files in commit1).
		for (int i = 0; i < c1->headFiles.len; ++i)
		{
			Diff diff;
			ConflictingStatus state = getConflictingStatus(&(c1->headFiles.arr[i]), c2->headFiles, &diff);
			String fpath = c1->headFiles.arr[i].file.path;

			switch (state)
			{
			case CONFLICT:
				freeDiffStruct(&diff);
			case SAME_BINARY:
			case SAME_TEXT:
			case REMOVED_IN_TARGET: // REMOVED_IN_TARGET (TARGET means c1 here -> our base)
				continue;			// handled in previous loop

			case NEW_FILE:		  // files in c1 that are not present in c2
			case REMOVED_IN_BASE: // files in c1 that are marked deleted in c2
				printf("\nFile " _CYANB "%s" _RST " is present in commit " _CYANB "'%s'" _RST ", but it is not found in commit " _YELB "'%s'" _RST ".\n", fpath, commit1HashStr, commit2HashStr);
				break; // handled later (in next for loop)
			}

			different = true;
		}
		freeCommitStruct(c1);
		freeCommitStruct(c2);

		if (!different)
			printf("\n" _GRNB "There is no difference between two commits!" _RST "\n");

		printf("\n");

		return ERR_NOERR;
	}
	else if (checkArgument(1, "-f")) // diff files !
	{
		int f1LineBoundsArgIndex = 0, f2LineBoundsArgIndex = 0;
		if (checkAnyArgument("-line1"))
			f1LineBoundsArgIndex = checkAnyArgument("-line1") + 1;
		if (checkAnyArgument("-line2"))
			f2LineBoundsArgIndex = checkAnyArgument("-line2") + 1;

		int f1End = -1, f1Begin = 1;
		int f2End = -1, f2Begin = 1;
		if (f1LineBoundsArgIndex > 4)
		{
			char tmp[10];
			strcpy(tmp, argv[f1LineBoundsArgIndex]);
			String tmp2[5];
			if (tokenizeString(tmp, "-", tmp2) != 2)
				return ERR_ARGS_MISSING;
			f1Begin = atoi(tmp2[0]);
			f1End = atoi(tmp2[1]);
			if (!f1Begin || !f1End)
				return ERR_ARGS_MISSING;
		}
		if (f2LineBoundsArgIndex > 4)
		{
			char tmp[10];
			strcpy(tmp, argv[f2LineBoundsArgIndex]);
			String tmp2[5];
			if (tokenizeString(tmp, "-", tmp2) != 2)
				return ERR_ARGS_MISSING;
			f2Begin = atoi(tmp2[0]);
			f2End = atoi(tmp2[1]);
			if (!f2Begin || !f2End)
				return ERR_ARGS_MISSING;
		}

		if (!performActions)
			return ERR_NOERR;
		if (!curRepository)
			return ERR_NOREPO;

		if (access(argv[2], F_OK) != 0)
			printError("File " _BOLD "%s" _UNBOLD " does not exist!", argv[2]);
		else if (access(argv[3], F_OK) != 0)
			printError("File " _BOLD "%s" _UNBOLD " does not exist!", argv[3]);
		else
		{
			withString(f1Path, normalizePath(argv[2], curRepository->absPath))
				withString(f2Path, normalizePath(argv[3], curRepository->absPath))
			{
				Diff diff = getDiff(argv[2], argv[3], f1Begin, f1End, f2Begin, f2End);
				printDiff(&diff, f1Path, f2Path);
				freeDiffStruct(&diff);
				return ERR_NOERR;
			}
		}
		return ERR_NOT_EXIST;
	}
	return ERR_ARGS_MISSING;
}

int command_merge(int argc, constString argv[], bool performActions)
{
	if (!checkArgument(1, "-b"))
		return ERR_ARGS_MISSING;
	if (!performActions)
		return ERR_NOERR;
	if (!curRepository)
		return ERR_NOREPO;

	constString baseBr = (argc == 4) ? argv[3] : curRepository->head.branch;
	constString mergingBr = argv[2];
	uint64_t base = getBranchHead(baseBr) & 0xFFFFFF;
	uint64_t merging = getBranchHead(mergingBr) & 0xFFFFFF;

	// Check if we have deatached  HEAD, then forbid to merge
	if (curRepository->deatachedHead)
	{
		printWarning("Your HEAD is in DEATACHED MODE! You can not merge...");
		return ERR_DEATACHED_HEAD;
	}

	// Check if the branches exist
	Commit *baseHeadCommit = getCommit(base);
	if (!baseHeadCommit)
	{
		printError("ERROR : Branch '%s' does not have any commits! (or not exist!)\n", baseBr);
		return ERR_NOT_EXIST;
	}
	Commit *mergingHeadCommit = getCommit(merging);
	if (!mergingHeadCommit)
	{
		printError("ERROR : Branch '%s' does not have any commits! (or not exist!)\n", mergingBr);
		freeCommitStruct(baseHeadCommit);
		return ERR_NOT_EXIST;
	}

	// Check the user.name and user.email
	String name = getConfig("user.name");
	String email = getConfig("user.email");
	if (!name || !email)
	{
		printError("You haven't been configured the user information!");
		printError("Please submit your information and configs for NeoGIT!\n");
		printWarning("You must first set them by Command(s) below:");
		if (!name)
			printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.name <yourname>\"" _RST);
		else
			free(name);
		if (!email)
			printWarning(_BOLD "\"" PROGRAM_NAME " config [--global] user.email <youremail@example.com>\"" _RST);
		else
			free(email);
		return ERR_CONFIG_NOTFOUND;
	}

	constString __c_b = curRepository->head.branch;				   // save temporarily current branch name
	int result = systemf("neogit checkout %s >/dev/null", baseBr); //  switch to the base branch
	fetchHEAD();												   // fetch head to update program structs

	GitObjectArray newObjects;
	newObjects.arr = NULL;
	newObjects.len = 0;

	if (result != ERR_NOERR)
	{
		_SET_ERR(result);
		goto __end;
	}

	// check if the merging branch is already merged ?
	String mergedDestination = getMergeDestination(mergingBr);
	if (mergedDestination)
	{
		printWarning("This branch is already merged to " _BOLD "'%s'" _UNBOLD ". You cannot merge this branch again. \n", mergedDestination);
		free((void *)mergedDestination);
		_SET_ERR(ERR_GENERAL);
		goto __end;
	}

	bool conflict = false;
	for (int i = 0; i < mergingHeadCommit->headFiles.len; ++i)
	{
		Diff diff;
		ConflictingStatus state = getConflictingStatus(&(mergingHeadCommit->headFiles.arr[i]), baseHeadCommit->headFiles, &diff);
		String fpath = mergingHeadCommit->headFiles.arr[i].file.path;

		switch (state)
		{
		case SAME_BINARY:
			continue;
		case SAME_TEXT:
			continue;
		case NEW_FILE:
			printf("New object added from branch " _CYANB "%s" _RST ": " _CYANB "%s" _RST "\n", mergingBr, fpath);
			ADD_EMPTY(newObjects.arr, newObjects.len, GitObject);
			newObjects.arr[newObjects.len - 1] = mergingHeadCommit->headFiles.arr[i];
			newObjects.arr[newObjects.len - 1].file.path = strDup(fpath);
			continue;
		case REMOVED_IN_BASE:
			if (!conflict)
				printf("\nConflicts are found while trying to merge:\n\n");
			conflict = true;
			printf("File " _BOLD "%s" _UNBOLD " is deleted in branch " _CYANB "'%s'" _RST ", but it is present in branch " _YELB "'%s'" _RST "\n", fpath, baseBr, mergingBr);
			break;
		case REMOVED_IN_TARGET:
			if (!conflict)
				printf("\nConflicts are found while trying to merge:\n\n");
			conflict = true;
			printf("File " _BOLD "%s" _UNBOLD " is present in branch " _CYANB "'%s'" _RST ", but it is deleted in branch " _YELB "'%s'" _RST "\n", fpath, baseBr, mergingBr);
			break;
		case CONFLICT:
			if (!conflict)
				printf("\nConflicts are found while trying to merge:\n\n");
			conflict = true;
			char str1[PATH_MAX], str2[PATH_MAX];
			sprintf(str1, "<%s>/%s", baseBr, fpath);
			sprintf(str2, "<%s>/%s", mergingBr, fpath);
			printDiff(&diff, str1, str2);
			freeDiffStruct(&diff);
			break;
		}
		printf("\n*****************************************\n\n");
	}

	if (conflict)
	{
		printError(_BOLD "\nConflicts are found! Merging canceled!\n" _UNBOLD);
		_SET_ERR(ERR_CONFLICT);
		goto __end;
	}

	else
	{
		char message[COMMIT_MSG_LEN_MAX];
		sprintf(message, "Merge branch '%s' into '%s'", mergingBr, baseBr);
		Commit *res = createCommit(&newObjects, name, email, message, mergingHeadCommit->hash);
		if (res)
		{
			printf("\nSuccessfully performed the merged: " _CYANB "'%s'\n" _RST, message);
			char datetime[DATETIME_STR_MAX];
			strftime(datetime, DATETIME_STR_MAX, DEFAULT_DATETIME_FORMAT, localtime(&res->time));
			printf("Date and Time : " _BOLD "%s\n" _RST, datetime);
			printf("Merge Commit Hash " _CYANB "'%06lx'\n\n" _RST, res->hash);

			// merge the head of merging branch to current head.
			setBranchHead(mergingBr, res->hash);

			// apply to working tree
			applyToWorkingDir(res->headFiles);

			// free commit strcut
			freeCommitStruct(res);

			// reset the error marker
			_RST_ERR;
		}
		else
		{
			printError("\nFailed to perform the merge.\n");
			_SET_ERR(ERR_GENERAL);
			goto __end;
		}
	}

__end:
	systemf("neogit checkout %s >/dev/null", __c_b); // checkout back to tmp saved branch
	fetchHEAD();									 // fetch head to update program structs
	free(name);
	free(email);
	freeCommitStruct(baseHeadCommit);
	freeCommitStruct(mergingHeadCommit);
	freeGitObjectArray(&newObjects);
	__retTry;
}
