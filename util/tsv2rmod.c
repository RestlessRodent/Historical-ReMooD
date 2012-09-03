#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char* Tok;
	FILE* inMI;
	
	/* Load State Tables */
	
	/* Run through object tables */
	inMI = fopen("her_mos.tsv", "rt");
	
	// Handle
	if (inMI)
	{
		// Constantly read
		while (!feof(inMI))
		{
			// Read string
			fgets(Buf, BUFSIZE, inMI);
			
			// Print
			fprintf(stderr, ".", Buf);
			
			// Tokenize
#define TOKSYMS " \t\r\n"
			for (Tok = strtok(Buf, TOKSYMS); Tok; Tok = strtok(NULL, TOKSYMS))
			{
			}
		}
	}
	
	/* Cleanup */
	fprintf(stderr, "\n");
	/* Done */
	return EXIT_SUCCESS;
#undef BUFSIZE
}

