#include "string_funcs.h"

String boldText(constString s)
{
	return strConcat(_BOLD, s, _UNBOLD);
}

int strReplace(String dest, constString text, constString wordPattern, String (*replaceFunction)(constString))
{
	int matchedWordCount = 0;
	// Duplicate the input text to avoid modifying the original string
	String textDup = strDup(text);
	// Initialize the destination string
	if (dest)
		dest[0] = '\0';

	// Tokenize the duplicated text
	String token = strtok(textDup, " \n\r\t");
	String lastToken = token;

	// Iterate through tokens
	for (token = strtok(NULL, " \n\r\t"); lastToken; lastToken = token, token = strtok(NULL, " \n\r\t"))
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

bool isEmpty(constString s)
{
	if (!s)
		return true; // If the String is null, return true
	uint len = 0;
	withString(ss, strtrim(strDup(s))) // Trim leading and trailing whitespaces
		len = strlen(ss);			   // Get the length of the trimmed string
	return (len == 0);				   // Return true if the trimmed string is empty, false otherwise
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
	if (src == NULL)
		return strDup("(null)");
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

String _static_strcat_impl(String dest, constString first, ...)
{
	va_list l;
	constString s;
	size_t cnt = 0;

	va_start(l, first);
	strcpy(dest, first);
	while (s = va_arg(l, String))
		strcat(dest, s);
	va_end(l);

	return dest;
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

String strptime(constString restrict s, constString restrict f, struct tm *restrict tm)
{
	int i, w, neg, adj, min, range, *dest, dummy;
	const char *ex;
	size_t len;
	int want_century = 0, century = 0, relyear = 0;
	while (*f)
	{
		if (*f != '%')
		{
			if (isspace(*f))
				for (; *s && isspace(*s); s++)
					;
			else if (*s != *f)
				return 0;
			else
				s++;
			f++;
			continue;
		}
		f++;
		if (*f == '+')
			f++;
		if (isdigit(*f))
		{
			char *new_f;
			w = strtoul(f, &new_f, 10);
			f = new_f;
		}
		else
		{
			w = -1;
		}
		adj = 0;
		switch (*f++)
		{
		case 'a':
		case 'A':
			dest = &tm->tm_wday;
			min = ABDAY_1;
			range = 7;
			goto symbolic_range;
		case 'b':
		case 'B':
		case 'h':
			dest = &tm->tm_mon;
			min = ABMON_1;
			range = 12;
			goto symbolic_range;
		case 'c':
			s = strptime(s, nl_langinfo(D_T_FMT), tm);
			if (!s)
				return 0;
			break;
		case 'C':
			dest = &century;
			if (w < 0)
				w = 2;
			want_century |= 2;
			goto numeric_digits;
		case 'd':
		case 'e':
			dest = &tm->tm_mday;
			min = 1;
			range = 31;
			goto numeric_range;
		case 'D':
			s = strptime(s, "%m/%d/%y", tm);
			if (!s)
				return 0;
			break;
		case 'H':
			dest = &tm->tm_hour;
			min = 0;
			range = 24;
			goto numeric_range;
		case 'I':
			dest = &tm->tm_hour;
			min = 1;
			range = 12;
			goto numeric_range;
		case 'j':
			dest = &tm->tm_yday;
			min = 1;
			range = 366;
			adj = 1;
			goto numeric_range;
		case 'm':
			dest = &tm->tm_mon;
			min = 1;
			range = 12;
			adj = 1;
			goto numeric_range;
		case 'M':
			dest = &tm->tm_min;
			min = 0;
			range = 60;
			goto numeric_range;
		case 'n':
		case 't':
			for (; *s && isspace(*s); s++)
				;
			break;
		case 'p':
			ex = nl_langinfo(AM_STR);
			len = strlen(ex);
			if (!strncasecmp(s, ex, len))
			{
				tm->tm_hour %= 12;
				s += len;
				break;
			}
			ex = nl_langinfo(PM_STR);
			len = strlen(ex);
			if (!strncasecmp(s, ex, len))
			{
				tm->tm_hour %= 12;
				tm->tm_hour += 12;
				s += len;
				break;
			}
			return 0;
		case 'r':
			s = strptime(s, nl_langinfo(T_FMT_AMPM), tm);
			if (!s)
				return 0;
			break;
		case 'R':
			s = strptime(s, "%H:%M", tm);
			if (!s)
				return 0;
			break;
		case 'S':
			dest = &tm->tm_sec;
			min = 0;
			range = 61;
			goto numeric_range;
		case 'T':
			s = strptime(s, "%H:%M:%S", tm);
			if (!s)
				return 0;
			break;
		case 'U':
		case 'W':
			/* Throw away result, for now. (FIXME?) */
			dest = &dummy;
			min = 0;
			range = 54;
			goto numeric_range;
		case 'w':
			dest = &tm->tm_wday;
			min = 0;
			range = 7;
			goto numeric_range;
		case 'x':
			s = strptime(s, nl_langinfo(D_FMT), tm);
			if (!s)
				return 0;
			break;
		case 'X':
			s = strptime(s, nl_langinfo(T_FMT), tm);
			if (!s)
				return 0;
			break;
		case 'y':
			dest = &relyear;
			w = 2;
			want_century |= 1;
			goto numeric_digits;
		case 'Y':
			dest = &tm->tm_year;
			if (w < 0)
				w = 4;
			adj = 1900;
			want_century = 0;
			goto numeric_digits;
		case '%':
			if (*s++ != '%')
				return 0;
			break;
		default:
			return 0;
		numeric_range:
			if (!isdigit(*s))
				return 0;
			*dest = 0;
			for (i = 1; i <= min + range && isdigit(*s); i *= 10)
				*dest = *dest * 10 + *s++ - '0';
			if (*dest - min >= (unsigned)range)
				return 0;
			*dest -= adj;
			switch ((char *)dest - (char *)tm)
			{
			case offsetof(struct tm, tm_yday):;
			}
			goto update;
		numeric_digits:
			neg = 0;
			if (*s == '+')
				s++;
			else if (*s == '-')
				neg = 1, s++;
			if (!isdigit(*s))
				return 0;
			for (*dest = i = 0; i < w && isdigit(*s); i++)
				*dest = *dest * 10 + *s++ - '0';
			if (neg)
				*dest = -*dest;
			*dest -= adj;
			goto update;
		symbolic_range:
			for (i = 2 * range - 1; i >= 0; i--)
			{
				ex = nl_langinfo(min + i);
				len = strlen(ex);
				if (strncasecmp(s, ex, len))
					continue;
				s += len;
				*dest = i % range;
				break;
			}
			if (i < 0)
				return 0;
			goto update;
		update:
			// FIXME
			;
		}
	}
	if (want_century)
	{
		tm->tm_year = relyear;
		if (want_century & 2)
			tm->tm_year += century * 100 - 1900;
		else if (tm->tm_year <= 68)
			tm->tm_year += 100;
	}
	return (char *)s;
}