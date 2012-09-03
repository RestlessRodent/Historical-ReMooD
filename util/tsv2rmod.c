#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* StateInfo_t -- Single State */
typedef struct StateInfo_s
{
	uint32_t DEHId;
	char* DEHName;
	char* Sprite;
	char* NextState;
	uint32_t NextID;
	char* Func;
	uint32_t Frame;
	uint32_t Tics;
} StateInfo_t;

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	char Buf[BUFSIZE];
	char* Tok;
	FILE* inMI, *inST;
	
	StateInfo_t* States;
	StateInfo_t* CurState;
	size_t NumStates, i, j;
	size_t CurStateNum, ReadNum;
	
	/* Load State Tables */
	inST = fopen("her_stt.tsv", "rt");
	
	// Handle
	if (inST)
	{
		// Clear
		States = NULL;
		NumStates = 0;
		CurState = 0;
		
		// Constantly read
		fprintf(stderr, "\nReading states");
		while (!feof(inST))
		{
			// Read string
			fgets(Buf, BUFSIZE, inST);
			
			// Print
			fprintf(stderr, ".", Buf);
			
			// Resize
			States = realloc(States, sizeof(*States) * (NumStates + 2));
			CurState = &States[CurStateNum = NumStates++];
			memset(CurState, 0, sizeof(*CurState));
			
			// DeHackEd ID
			CurState->DEHId = CurStateNum;
			
			// Tokenize
#define TOKSYMS " \t\r\n"
			ReadNum = 0;
			for (Tok = strtok(Buf, TOKSYMS); Tok; Tok = strtok(NULL, TOKSYMS))
				// Which read number?
				switch (ReadNum++)
				{
						// Name
					case 0:
						CurState->DEHName = strdup(Tok);
						break;
					
						// Sprite
					case 1:
						CurState->Sprite = strdup(Tok + 4);
						break;
						
						// Frame
					case 2:
						CurState->Frame = strtol(Tok, NULL, 0);
						break;
						
						// Tics
					case 3:
						CurState->Tics = strtol(Tok, NULL, 0);
						break;
						
						// Function
					case 4:
						CurState->Func = strdup(Tok);
						break;
						
						// Next State
					case 5:
						CurState->NextState = strdup(Tok);
						break;
						
					default:
						break;
				}
		}
	}
	fprintf(stderr, "done!\n");
	
	/* Map next states to IDs */
	fprintf(stderr, "Linking IDs");
	for (i = 0; i < NumStates; i++)
		for (j = 0; j < NumStates; j++)
			if (States[i].NextState && States[j].DEHName)
				if (strcasecmp(States[i].NextState, States[j].DEHName) == 0)
				{
					fprintf(stderr, ".");
					States[i].NextID = j;
					break;
				}
	fprintf(stderr, "done!\n");
	
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

