/***************
*** INCLUDES ***
***************/

#include "c_lib.h"

/****************
*** FUNCTIONS ***
****************/

/* C_strupr() -- Uppercase characters */
char* C_strupr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
	
	/* Run */
	x = s;
	while (*x)
	{
		*x = toupper(*x);
		x++;
	}
	
	return s;
}

/* C_strlwr() -- Lowercase characters */
char* C_strlwr(char* s)
{
	char* x;
	
	/* Check */
	if (!s)
		return NULL;
	
	/* Run */
	x = s;
	while (*x)
	{
		*x = tolower(*x);
		x++;
	}
	
	return s;
}

