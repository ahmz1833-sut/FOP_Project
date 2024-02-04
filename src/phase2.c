/*******************************
 *         phase2.c            *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "phase2.h"

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
		targetStr = argv[2];
		noCommit = true;
	}

	uint mIdx = checkAnyArgument("-m"); // check for switch -m (change message)
	if (mIdx == 2 || mIdx == 1)
	{
		if (argc != 4)
			return ERR_ARGS_MISSING;

		targetStr = (mIdx == 1) ? argv[3] : argv[1];
		message = argv[mIdx + 1];
	}
	else if(mIdx)
	{
		if (argc != 2)
			return ERR_ARGS_MISSING;
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
}

int command_grep(int argc, constString argv[], bool performActions)
{
}

int command_diff(int argc, constString argv[], bool performActions)
{
	if (checkArgument(1, "-c")) // diff commits!
	{
		if (argc != 5)
			return ERR_ARGS_MISSING;
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
					printf("File " _CYAN _BOLD "%s" _UNBOLD _DFCOLOR " vs File " _YEL _BOLD "%s" _UNBOLD _DFCOLOR " :\n", f1Path, f2Path);

			Diff diff = getDiff(argv[2], argv[3], f1Begin, f1End, f2Begin, f2End);
			if (diff.addedCount != 0 || diff.removedCount != 0)
			{
				printf("<<<<<<<<<\n");
				int i = 0;
				for (i = 0; i < diff.removedCount && i < diff.addedCount; i++)
				{
					printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", getFileName(argv[2]), diff.lineNumberRemoved[i]);
					printf(_DIM "< " _RST _CYAN "%s\n" _RST, diff.linesRemoved[i]);
					printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", getFileName(argv[3]), diff.lineNumberAdded[i]);
					printf(_DIM "> " _RST _YEL "%s\n" _RST, diff.linesAdded[i]);
				}
				for (; i < diff.removedCount; i++)
				{
					printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", getFileName(argv[2]), diff.lineNumberRemoved[i]);
					printf(_DIM "< " _RST _CYAN "%s\n" _RST, diff.linesRemoved[i]);
				}
				for (; i < diff.addedCount; i++)
				{
					printf(_DIM "file " _BOLD "%s" _UNBOLD _DIM " - line %d\n", getFileName(argv[3]), diff.lineNumberAdded[i]);
					printf(_DIM "> " _RST _YEL "%s\n" _RST, diff.linesAdded[i]);
				}
				printf(">>>>>>>>>\n");
				freeDiffStruct(&diff);
			}
			else
				printf(_GRN "No Difference found!\n" _RST);
			return ERR_NOERR;
		}
		return ERR_NOT_EXIST;
	}
	return ERR_ARGS_MISSING;
}

int command_merge(int argc, constString argv[], bool performActions)
{
}