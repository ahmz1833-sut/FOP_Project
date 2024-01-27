#include "string_funcs.h"

bool isEmpty(constString s)
{
    if(!s) return true;             // If the String is null, return true
    String ss = strtrim(strDup(s)); // Trim leading and trailing whitespaces
    uint len = strlen(ss);          // Get the length of the trimmed string
    free(ss);                       // Free the memory allocated for the trimmed string
    return (len == 0);              // Return true if the trimmed string is empty, false otherwise
}

String strReplace(String str, char find, char replace)
{
    char *current_pos = strchr(str, find);
    while (current_pos)
    {
        *current_pos = replace;
        current_pos = strchr(current_pos, find);
    }
    return str;
}

uint strValidate(String dest, constString str, constString allowedChars)
{
    if (str == NULL || allowedChars == NULL)
        return 0;

    size_t len = strlen(str);
    size_t validCount = 0;

    unsigned isInclusive = 1;
    if (allowedChars[0] == '^')
    {
        isInclusive = 0;
        ++allowedChars; // Skip the '^'
    }

    for (size_t i = 0; i < len; ++i)
    {
        int isInRange = 0;
        constString targetChars = allowedChars;
        while (*targetChars && *targetChars != ']')
        {
            if (*(targetChars + 1) == '-' && *(targetChars + 2) && *(targetChars + 2) != ']')
            {
                // Range indicator '-' inside brackets
                char startChar = *targetChars;
                char endChar = *(targetChars + 2);

                if (startChar < endChar && str[i] >= startChar && str[i] <= endChar)
                {
                    isInRange = 1;
                    break;
                }

                targetChars += 3; // Skip the whole range part
            }
            else
            {
                // Single character
                if (str[i] == *targetChars)
                {
                    isInRange = 1;
                    break;
                }
                ++targetChars;
            }
        }

        if (!(isInclusive ^ isInRange) && str[i] != '[' && str[i] != ']')
        {
            if (dest != NULL)
                dest[validCount] = str[i];

            validCount++;
        }
    }

    // Null-terminate the resulting string
    if (dest != NULL)
        dest[validCount] = '\0';

    return len - validCount;
}

String strtrim(String s)
{
    while (*s && strchr(" \r\t\n\f", *s))
        memmove(s, s + 1, strlen(s));
    while (*s && strchr(" \r\t\n\f", s[strlen(s) - 1]))
        s[strlen(s) - 1] = '\0';
    return s;
}

uint tokenizeString(String str, constString delim, String *destArray)
{
    int count = 0;
    destArray[0] = strtok(str, delim);
    while (destArray[count])
        destArray[++count] = strtok(NULL, delim);
    return count;
}

String strDup(constString src)
{
    if(src == NULL) return strDup("(null)");
    String dest = malloc(strlen(src) + 1);
    strcpy(dest, src);
    return dest;
}

String better_strcat_impl(constString first, ...)
{
    va_list l;
    constString s;
    size_t cnt = 0;

    va_start(l, first);
    cnt += strlen(first);
    while (s = va_arg(l, String))
        cnt += strlen(s);
    va_end(l);

    String ans = malloc(cnt + 1);
    strcpy(ans, first);
    cnt = strlen(first);
    va_start(l, first);
    while (s = va_arg(l, String))
    {
        strcpy(ans + cnt, s);
        cnt += strlen(s);
    }
    va_end(l);

    return ans;
}

bool isMatch(constString _text, constString _pattern)
{
    if (_text == NULL || _pattern == NULL)
        return false;

    String text = strtrim(strDup(_text));
    String pattern = strtrim(strDup(_pattern));

    int n = strlen(text);
    int m = strlen(pattern);
    int i = 0, j = 0, startIndex = -1, match = 0;

    while (i < n)
    {
        if (j < m && (pattern[j] == '?' || pattern[j] == text[i]))
        {
            // Characters match or '?' in pattern matches any character.
            i++;
            j++;
        }
        else if (j < m && pattern[j] == '*')
        {
            // Wildcard character '*', mark the current position in the pattern and the text as a proper match.
            startIndex = j;
            match = i;
            j++;
        }
        else if (startIndex != -1)
        {
            // No match, but a previous wildcard was found. Backtrack to the last '*' character position and try for a different match.
            j = startIndex + 1;
            match++;
            i = match;
        }
        else
        {
            // If none of the above cases comply, the pattern does not match.
            return false;
        }
    }

    // Consume any remaining '*' characters in the given pattern.
    while (j < m && pattern[j] == '*')
        j++;

    free(pattern);
    free(text);

    // If we have reached the end of both the pattern and the text, the pattern matches the text.
    return j == m;
}