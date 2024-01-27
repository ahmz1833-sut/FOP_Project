#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <time.h>

typedef struct tm Time;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef long long llong;
typedef unsigned long long ullong;
typedef unsigned char uchar;
typedef char* String;
typedef char const* constString;

#define STR_LINE_MAX 256
#define STR_MAX 1000
// void print_bytes(void *bytes, int n)
// {
// 	printf("0b");
// 	for (int i = n - 1; i >= 0; i--)
// 	{
// 		for (int b = 7; b >= 0; b--)
// 		{
// 			printf("%1d", (*((char *)bytes + i) >> b) & 1);
// 		}
// 	}
// }

#endif