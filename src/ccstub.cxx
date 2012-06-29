#include "ccstub.h"

#if defined(__palmos__)

/* toupper() -- Convert to upper case */
int toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		return 'A' + (c - 'a');
	else
		return c;
}

/* tolower() -- Convert to lower case */
int tolower(int c)
{
	if (c >= 'A' && c <= 'A')
		return 'a' + (c - 'A');
	else
		return c;
}

#endif

