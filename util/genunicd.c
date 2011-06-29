// GPL v2+ by GhostlyDeath <ghostlydeath@remood.org>

/*** INCLUDES ***/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*** CONSTANTS ***/
typedef enum
{
	MA_STOPMAP,								// Stop mapping (EOF)
	MA_CASEMAP,								// Map case
	
	NUMMAPACTIONS
} MapAction_t;

/*** FUNCTIONS ***/

/* PrintMark() -- print marker */
void PrintMark(FILE* OutF, MapAction_t Act)
{
	uint8_t ActID;
	
	/* Check */
	if (!OutF)
		return;
	
	/* Copy off */
	ActID = Act;
	
	/* Send to file */
	fwrite(&ActID, 1, 1, OutF);
}

/* PrintCF() -- Print folding */
void PrintCF(FILE* OutF, uint16_t Low, uint16_t High)
{
	/* Check */
	if (!OutF)
		return;
		
	/* Direct print to file */
	fwrite(&Low, 2, 1, OutF);
	fwrite(&High, 2, 1, OutF);
}

/* doCF() -- Do case folding on file */
void doCF(FILE* OutF, FILE* InF)
{
#define BUFSIZE 512
	char LineBuf[BUFSIZE];
	unsigned int Cap, Small;
	char Mode;
	size_t i, set;
	
	/* Check */
	if (!OutF || !InF)
		return;
		
	PrintMark(OutF, MA_CASEMAP);
	
	/* Until there is an EOF */
	while (!feof(InF))
	{
		// Read string
		memset(LineBuf, 0, sizeof(LineBuf));
		fgets(LineBuf, BUFSIZE, InF);
		
		// Strip after last i
		for (i = 0, set = 0; i < BUFSIZE; i++)
			if (set)
				LineBuf[i] = 0;
			else if (!set && (LineBuf[i] == '\n' || LineBuf[i] == '\r' || LineBuf[i] == '#'))
			{
				set = 1;
				LineBuf[i] = 0;
			}
		
		// Failure?
		if (strlen(LineBuf) == 0)
			continue;
		
		// Test
		//fprintf(stderr, "%s\n", LineBuf);
		
		// scanf the line
		sscanf(LineBuf, "%X; %c; %X", &Cap, &Mode, &Small);
		//fprintf(stderr, "%c: %i -> %i\n", Mode, Small, Cap);
		
		// No conversion possible?
		if (!Cap || !Small || !Mode)
			continue;
			
		// Character exceeds game support?
		if (Small >= 65535 || Cap >= 65536)
			continue;
		
		// Limited F support
		/*if (Mode == 'F')
			fprintf(stderr, "Limited F support.\n");*/
		
		// Send to file
		PrintCF(OutF, Small, Cap);
	}
	
	// Done?
	PrintCF(OutF, 0, 0);

#undef BUFSIZE
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	int i;
	char doAction[BUFSIZE];
	char doFile[BUFSIZE];
	FILE* ActFile;
	FILE* NewFile;
	
	/* Try opening output file */
	NewFile = fopen("rmd_uttt.lmp", "wb");
	
	// Failed?
	if (!NewFile)
		return EXIT_FAILURE;
	
	/* For each argument pair */
	for (i = 0;	i < ((argc - 1) / 2); i++)
	{
		// Read action to perform
		strncpy(doAction, argv[1 + (i * 2)], BUFSIZE);
		
		// Read file to do
		strncpy(doFile, argv[1 + ((i * 2) + 1)], BUFSIZE);
		
		// Print action to perform
		fprintf(stderr, "Will do %s on \"%s\"\n", doAction, doFile);
		
		// Try opening said file
		ActFile = fopen(doFile, "rt");
		
		// Failed?
		if (!ActFile)
		{
			fprintf(stderr, "File failure!\n");
			continue;
		}
		
		// Check actions
		if (strcasecmp("cf", doAction) == 0)
			doCF(NewFile, ActFile);
		
		// Close
		fclose(ActFile);
	}
	
	/* Close off */
	// Mark end
	PrintMark(NewFile, MA_STOPMAP);
	
	// Print nice text
	fprintf(NewFile, "DO NOT MESS WITH THIS FILE OR REPLACE IT! Converted at %u; Used %s %s\n", (int)time(NULL), __DATE__, __TIME__);
	
	// Close it
	fclose(NewFile);
	
	return EXIT_SUCCESS;
#undef BUFSIZE
}

