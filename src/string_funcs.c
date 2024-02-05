/*******************************
 *      string_funcs.c         *
 *    Copyright 2024 AHMZ      *
 *  AmirHossein MohammadZadeh  *
 *         402106434           *
 *     FOP Project NeoGIT      *
 ********************************/
#include "string_funcs.h"

String boldText(constString s)
{
	return strcat_d(_BOLD, s, _UNBOLD);
}

String boldAndUnderlineText(constString s)
{
	return strcat_d(_BOLD _UNDERL, s, _UNBOLD _NOUNDERL);
}

String strtrim(String s)
{
	while (*s && strchr(" \r\t\n\f", *s))
		memmove(s, s + 1, strlen(s));
	while (*s && strchr(" \r\t\n\f", s[strlen(s) - 1]))
		s[strlen(s) - 1] = '\0';
	return s;
}

bool isEmpty(constString s)
{
	if (!s)
		return true; // If the String is null, return true
	uint len = 0;
	withString(ss, strtrim(strDup(s))) // Trim leading and trailing whitespaces
		len = strlen(ss);			   // Get the length of the trimmed string
	return (len == 0);				   // Return true if the trimmed string is empty, false otherwise
}

String strDup(constString src)
{
	if (src == NULL)
		return strDup("(null)");
	String dest = malloc(strlen(src) + 1);
	strcpy(dest, src);
	return dest;
}

int strReplace(String dest, constString text, constString _wordPattern, String (*replaceFunction)(constString))
{
	int matchedWordCount = 0;

	char wordPattern[STR_MAX];
	strValidate(wordPattern, _wordPattern, "^" SEARCH_DELIMETERS);

	// Duplicate the input text to avoid modifying the original string
	String textDup = strDup(text);
	// Initialize the destination string
	if (dest)
		dest[0] = '\0';

	// Tokenize the duplicated text
	String token = strtok(textDup, SEARCH_DELIMETERS);
	String lastToken = token;

	// Iterate through tokens
	for (token = strtok(NULL, SEARCH_DELIMETERS); lastToken; lastToken = token, token = strtok(NULL, " \n\r\t"))
	{
		if (isMatch(lastToken, wordPattern))
		{
			// Matched a word
			matchedWordCount++;
			if (replaceFunction && dest)
			{
				// Replace the word using the provided function
				withString(newWord, replaceFunction(lastToken))
					strcat(dest, newWord); // Append replaced word to destination
			}
		}
		else
		{
			// Copy the last token to the destination
			if (dest)
				strcat(dest, lastToken);
		}

		if (!dest)
			continue;

		if (token)
		{
			// Append the substring between lastToken and token to the destination
			int ii = (token - lastToken) - strlen(lastToken) + strlen(dest);
			strncat(dest, text + (lastToken - textDup) + strlen(lastToken), ii - strlen(dest));
			dest[ii] = '\0';
		}
		else
		{
			// Append the remaining substring to the destination
			strcat(dest, text + (lastToken - textDup) + strlen(lastToken));
		}
	}

	// Free the duplicated text
	free(textDup);
	return matchedWordCount;
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

String toHexString(ullong number, int digits)
{
	String uniqueString = malloc(digits + 1);
	char format[10];
	sprintf(format, "%%0%dllX", digits);
	sprintf(uniqueString, format, number);
	return uniqueString;
}

uint tokenizeString(String str, constString delim, String *destArray)
{
	int count = 0;
	destArray[0] = strtok(str, delim);
	while (destArray[count])
		destArray[++count] = strtok(NULL, delim);
	return count;
}

bool isMatch(constString _text, constString _pattern)
{
	if (_text == NULL || _pattern == NULL)
		return false;

	int i = 0, j = 0, n = 0, m = 0;
	withString(text, strtrim(strDup(_text)))
	{
		withString(pattern, strtrim(strDup(_pattern)))
		{
			n = strlen(text);
			m = strlen(pattern);
			int startIndex = -1, match = 0;

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
					free(text);
					free(pattern);
					return false;
				}
			}

			// Consume any remaining '*' characters in the given pattern.
			while (j < m && pattern[j] == '*')
				j++;
		}
	}

	// If we have reached the end of both the pattern and the text, the pattern matches the text.
	return j == m;
}

time_t parseDateTimeAuto(constString dateTimeStr)
{
	struct tm tm = {0};
	// Try parsing in format "YYYY-MM-DD HH:MM:SS"
	if (strptime(dateTimeStr, "%Y-%m-%d %H:%M:%S", &tm) != NULL)
		return mktime(&tm);
	// Try parsing in format "YYYY/MM/DD HH:MM:SS"
	if (strptime(dateTimeStr, "%Y/%m/%d %H:%M:%S", &tm) != NULL)
		return mktime(&tm);
	// Try parsing in format "YYYY-MM-DD"
	if (strptime(dateTimeStr, "%Y-%m-%d", &tm) != NULL)
		return mktime(&tm);
	// Try parsing in format "YYYY/MM/DD"
	if (strptime(dateTimeStr, "%Y/%m/%d", &tm) != NULL)
		return mktime(&tm);
	// Error!
	return ERR_ARGS_MISSING;
}

String _dynamic_strcat_impl(constString first, ...)
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

String _static_strcat_impl(String dest, constString first, ...)
{
	va_list l;
	constString s;

	va_start(l, first);
	strcpy(dest, first);
	while (s = va_arg(l, String))
		strcat(dest, s);
	va_end(l);

	return dest;
}