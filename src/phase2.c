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