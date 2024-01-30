#include "hash.h"

ullong djb2Hash(constString str)
{
    ullong hash = rand() * rand();
    int c;
    while (c = *str++)
        hash = (((hash << (c % 16))) + hash) + (rand() % c);
    return hash;
}

ullong generateUniqueId(int hexdigits)
{
    time_t currentTime = time(NULL);
    char timeString[20];
    sprintf(timeString, "%ld", currentTime);

    ullong hashValue = djb2Hash(timeString);
    if (hexdigits < 16)
        hashValue &= (1ULL << (hexdigits * 4)) - 1;
    return hashValue;
}

String toHexString(ullong number, int digits)
{
    String uniqueString = malloc(digits + 1);
    char format[10];
    sprintf(format, "%%0%dllX", digits);
    sprintf(uniqueString, format, number);
    return uniqueString;
}