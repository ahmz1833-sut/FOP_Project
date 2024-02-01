#ifndef __PUSHPOP_H__
#define __PUSHPOP_H__

#include "neogit.h"

int backupStagingArea();
int restoreStageingBackup();
int popStage();
int popHead();

#endif