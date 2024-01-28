#ifndef __HASH_H__
#define __HASH_H__

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

ullong generateUniqueId(int hexdigits);
String toHexString(ullong number, int digits);

#endif