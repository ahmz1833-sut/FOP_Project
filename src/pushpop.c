#include "pushpop.h"

extern Repository *curRepository;

int restoreStageingBackup()
{
	char tmp[PATH_MAX];
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old0", curRepository->absPath);
	if (access(tmp, F_OK) == -1)
		return ERR_NOT_EXIST; // check if not exist

	FileEntry *buf = NULL;
	strConcatStatic(tmp, curRepository->absPath, "/." PROGRAM_NAME "/stage");
	int res = ls(&buf, tmp);
	int error = 0;
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			remove(buf[i].path);
	freeFileEntry(buf, res);

	// remove the top stack
	char stagePath[PATH_MAX];
	strcpy(stagePath, tmp);
	strcat(tmp, "/old0");
	systemf("mv \"%s\"/* \"%s\" 2>/dev/null", tmp, stagePath); // move back all backed up files
	systemf("rm -r \"%s\"", tmp);							   // remove backup folder

	for (int i = 1; i <= 9; i++)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);
		// check if not exist
		if (access(tmp, F_OK) == -1)
			break;

		// move top
		tmp[strlen(tmp) - 1] = '\0'; // truncate and remove number from end of filename
		systemf("mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i - 1);
	}
	return ERR_NOERR;
}

int backupStagingArea()
{
	char tmp[PATH_MAX];
	for (int i = 9; i >= 0; i--)
	{
		sprintf(tmp, "%s/." PROGRAM_NAME "/stage/old%d", curRepository->absPath, i);
		// check if exist
		if (access(tmp, F_OK) == -1)
			continue;

		// move to newer one
		char cmd[PATH_MAX * 2 + 10];
		tmp[strlen(tmp) - 1] = '\0'; // truncate and remove number from end of filename
		if (i == 9)
			sprintf(cmd, "rm -r \"%s%d\"", tmp, i);
		else
			sprintf(cmd, "mv \"%s%d\" \"%s%d\"", tmp, i, tmp, i + 1);

		system(cmd);
	}
	sprintf(tmp, "%s/." PROGRAM_NAME "/stage", curRepository->absPath);
	FileEntry *buf = NULL;
	int res = ls(&buf, tmp);
	int error = 0;
	strcat(tmp, "/old0");
	mkdir(tmp, 0775);
	for (int i = 0; i < res; i++)
		if (!buf[i].isDir)
			withString(dest, strConcat(tmp, "/", getFileName(buf[i].path)))
				error += copyFile(buf[i].path, dest, "");
	freeFileEntry(buf, res);
	return error;
}

int popStage()
{
	char path[PATH_MAX];

	if (curRepository->stagingArea.arr)
	{
		free(curRepository->stagingArea.arr);
		curRepository->stagingArea.arr = NULL;
		curRepository->stagingArea.len = 0;
	}

	tryWithFile(infoFile, strConcatStatic(path, curRepository->absPath, "/." PROGRAM_NAME "/stage/info"),
				({ return ERR_FILE_ERROR; }), ({ return _ERR; }))
	{
		char buf[STR_LINE_MAX];
		int error = 0;
		while (fgets(buf, STR_LINE_MAX, infoFile) != NULL)
		{
			char filePath[PATH_MAX];
			time_t timeM = 0;
			uint perm = 0;
			char hash[11];
			sscanf(buf, "%[^:]:%ld:%u:%s", filePath, &timeM, &perm, hash);

			StagedFile *sf = getStagedFile(filePath);
			if (sf == NULL)
			{
				ADD_EMPTY(curRepository->stagingArea, StagedFile);
				sf = &(curRepository->stagingArea.arr[curRepository->stagingArea.len - 1]);
				sf->file.path = strDup(filePath);
			}
			strcpy(sf->hashStr, hash);
			sf->file.dateModif = timeM;
			sf->file.isDeleted = (strcmp("dddddddddd", hash) == 0);
			sf->file.permission = perm;
		}
		rewind(infoFile);
	}
}

int popHead()
{
	char path[PATH_MAX];
	curRepository->deatachedHead = false;
	curRepository->head.branch = "master";
	curRepository->head.hash = 0xFFFFFF;
	curRepository->head.headFiles.arr = NULL;
	curRepository->head.headFiles.len = 0;

	strConcatStatic(path, curRepository->absPath, "/.neogit/HEAD");
	systemf("touch \"%s\"", path);

	char HEAD_content[STR_LINE_MAX] = {0};
	tryWithFile(HEADfile, path, ({ return ERR_FILE_ERROR; }), ({ return _ERR; }))
		fgets(HEAD_content, STR_LINE_MAX, HEADfile);

	Commit *head = NULL;
	if (isMatch(HEAD_content, "branch/*"))
	{
		char branch[STR_LINE_MAX];
		sscanf(HEAD_content, "branch/%s", branch);
		curRepository->head.branch = strDup(branch);
		curRepository->head.hash = getBranchHead(branch);
		head = getCommit(curRepository->head.hash);
	}
	else if (isMatch(HEAD_content, "commit/*"))
	{
		curRepository->deatachedHead = true;
		sscanf(HEAD_content, "commit/%lx", &curRepository->head.hash);
		head = getCommit(curRepository->head.hash);
		if (head)
			curRepository->head.branch = head->branch;
	}
	else
		systemf("echo branch/master>\"%s/.neogit/HEAD\"", curRepository->absPath);

	if (head)
	{
		// obtain head files
		curRepository->head.headFiles = head->headFiles;

		if (head->username)
			free(head->username);
		if (head->useremail)
			free(head->useremail);
		if (head->message)
			free(head->message);
		if (head->commitedFiles.arr)
			free(head->commitedFiles.arr);
		// We don't free "branch" and "headFiles arr" (we need them)
		free(head);
	}
	return ERR_NOERR;
}