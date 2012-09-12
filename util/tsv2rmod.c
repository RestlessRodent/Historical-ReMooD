#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TOKSYMS " \t\r\n"

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

/* MobjInfo_t -- Object info */
typedef struct MobjInfo_s
{
	uint32_t DEHId;
	int32_t DoomEdNum;
	char* ClassName;
	char* MTName;
	char* SpawnState;
	uint32_t SpawnStateID;
	int32_t SpawnHealth;
	char* WakeState;
	uint32_t WakeStateID;
	char* WakeSound;
	int32_t ReactionTime;
	char* AttackSound;
	char* PainState;
	uint32_t PainStateID;
	int32_t PainChance;
	char* PainSound;
	char* MeleeState;
	uint32_t MeleeStateID;
	char* MissileState;
	uint32_t MissileStateID;
	char* CrashState;
	uint32_t CrashStateID;
	char* DeathState;
	uint32_t DeathStateID;
	char* XDeathState;
	uint32_t XDeathStateID;
	char* DeathSound;
	int32_t Speed;
	int32_t Radius;
	int32_t Height;
	int32_t Mass;
	int32_t Damage;
	char* ActiveSound;
	char* Flags;
	char* RaiseState;
	uint32_t RaiseStateID;
	char* FlagsTwo;
} MobjInfo_t;

static StateInfo_t* l_States;					// State List
static size_t l_NumStates;

static MobjInfo_t* l_Infos;						// Objects
static size_t l_NumInfos;

/* StateForName() -- Locates state for name */
uint32_t StateForName(const char* const a_Name)
{
	uint32_t i;
	
	/* Check */
	if (!a_Name)
		return 0;
	
	/* Loop */
	for (i = 0; i < l_NumStates; i++)
		if (l_States[i].DEHName)
			if (strcasecmp(a_Name, l_States[i].DEHName) == 0)
				return i;
	
	/* Not found */
	return 0;
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	char Buf[BUFSIZE], BufB[BUFSIZE];
	char* Tok;
	FILE* inMI, *inST, *inNM;
	
	StateInfo_t* CurState;
	size_t i, j;
	size_t CurStateNum, ReadNum;
	
	MobjInfo_t* CurInfo;
	size_t CurInfoNum;
	
	/* Load State Tables */
	inST = fopen("her_stt.tsv", "rt");
	
	// Handle
	if (inST)
	{
		// Clear
		l_States = NULL;
		l_NumStates = 0;
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
			l_States = realloc(l_States, sizeof(*l_States) * (l_NumStates + 2));
			CurState = &l_States[CurStateNum = l_NumStates++];
			memset(CurState, 0, sizeof(*CurState));
			
			// DeHackEd ID
			CurState->DEHId = CurStateNum;
			
			// Tokenize
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
	for (i = 0; i < l_NumStates; i++)
	{
		fprintf(stderr, ".");
		l_States[i].NextID = StateForName(l_States[i].NextState);
	}
	fprintf(stderr, "done!\n");
	
	/* Run through object tables */
	inMI = fopen("her_mos.tsv", "rt");
	inNM = fopen("her_name.tsv", "rt");
	
	// Handle
	if (inMI)
	{
		fprintf(stderr, "Parsing things");
		
		// Constantly read
		while (!feof(inMI))
		{
			// Read string
			fgets(Buf, BUFSIZE, inMI);
			
			// Print
			fprintf(stderr, ".", Buf);
			
			// Resize
			l_Infos = realloc(l_Infos, sizeof(*l_Infos) * (l_NumInfos + 2));
			CurInfo = &l_Infos[CurInfoNum = l_NumInfos++];
			memset(CurInfo, 0, sizeof(*CurInfo));
			
			// DeHackEd ID
			CurInfo->DEHId = CurInfoNum;
			CurInfo->DoomEdNum = -1;
			
			// Read Name
			if (inNM)
				if (!feof(inNM))
				{
					fgets(BufB, BUFSIZE, inNM);
					Tok = strtok(BufB, TOKSYMS);
					if (Tok)
						CurInfo->ClassName = strdup(Tok);
				}
				else
					CurInfo->ClassName = strdup("Whoops!");
			
			// Tokenize
			ReadNum = 0;
			for (Tok = strtok(Buf, TOKSYMS); Tok; Tok = strtok(NULL, TOKSYMS))
				switch (ReadNum++)
				{
						// MT Name
					case 0:
						CurInfo->MTName = strdup(Tok);
						break;
						
						// DoomEdNum
					case 1:
						CurInfo->DoomEdNum = strtol(Tok, NULL, 0);
						break;
						
						// Spawn State
					case 2:
						CurInfo->SpawnState = strdup(Tok);
						CurInfo->SpawnStateID = StateForName(CurInfo->SpawnState);
						break;
						
						// Spawn Health
					case 3:
						CurInfo->SpawnHealth= strtol(Tok, NULL, 0);
						break;
					
						// See State
					case 4:
						CurInfo->WakeState = strdup(Tok);
						CurInfo->WakeStateID = StateForName(CurInfo->WakeState);
						break;
						
						// See Sound
					case 5:
						CurInfo->WakeSound = strdup(Tok + 4);
						break;
						
						// Reaction Time
					case 6:
						CurInfo->ReactionTime = strtol(Tok, NULL, 0);
						break;
						
						// Attack Sound
					case 7:
						CurInfo->AttackSound = strdup(Tok + 4);
						break;
						
						// Pain State
					case 8:
						CurInfo->PainState = strdup(Tok);
						CurInfo->PainStateID = StateForName(CurInfo->PainState);
						break;
						
						// Pain Chance
					case 9:
						CurInfo->PainChance = strtol(Tok, NULL, 0);
						break;
						
						// Pain Sound
					case 10:
						CurInfo->PainSound = strdup(Tok + 4);
						break;
						
						// Melee State
					case 11:
						CurInfo->MeleeState = strdup(Tok);
						CurInfo->MeleeStateID = StateForName(CurInfo->MeleeState);
						break;
						
						// Missile State
					case 12:
						CurInfo->MissileState = strdup(Tok);
						CurInfo->MissileStateID = StateForName(CurInfo->MissileState);
						break;
						
						// Crash State
					case 13:
						CurInfo->CrashState = strdup(Tok);
						CurInfo->CrashStateID = StateForName(CurInfo->CrashState);
						break;
						
						// Death State
					case 14:
						CurInfo->DeathState = strdup(Tok);
						CurInfo->DeathStateID = StateForName(CurInfo->DeathState);
						break;
						
						// XDeath State
					case 15:
						CurInfo->XDeathState = strdup(Tok);
						CurInfo->XDeathStateID = StateForName(CurInfo->XDeathState);
						break;
						
						// Death Sound
					case 16:
						CurInfo->DeathSound = strdup(Tok + 4);
						break;
						
						// Speed
					case 17:
						CurInfo->Speed = strtol(Tok, NULL, 0);
						break;
						
						// Radius
					case 18:
						CurInfo->Radius = strtol(Tok, NULL, 0);
						break;
						
						// Height
					case 19:
						CurInfo->Height = strtol(Tok, NULL, 0);
						break;
						
						// Mass
					case 20:
						CurInfo->Mass = strtol(Tok, NULL, 0);
						break;
						
						// Damage
					case 21:
						CurInfo->Damage = strtol(Tok, NULL, 0);
						break;
						
						// Active Sound
					case 22:
						CurInfo->ActiveSound = strdup(Tok + 4);
						break;
						
						// Flags
					case 23:
						CurInfo->Flags = strdup(Tok);
						break;
						
						// Raise State
					case 24:
						CurInfo->RaiseState = strdup(Tok);
						CurInfo->RaiseStateID = StateForName(CurInfo->RaiseState);
						break;
						
						// Flags 2
					case 25:
						CurInfo->FlagsTwo = strdup(Tok);
						break;
						
						// Unknown
					default:
						break;
				}
				
				
	int32_t ReactionTime;
	char* AttackSound;
	char* PainState;
	uint32_t PainStateID;
	int32_t PainChance;
	char* PainSound;
	char* MeleeState;
	uint32_t MeleeStateID;
	char* MissileState;
	uint32_t MissileStateID;
	char* CrashState;
	uint32_t CrashStateID;
	char* DeathState;
	uint32_t DeathStateID;
	char* XDeathState;
	uint32_t XDeathStateID;
	char* DeathSound;
	int32_t Speed;
	int32_t Radius;
	int32_t Height;
	int32_t Mass;
	int32_t Damage;
	char* ActiveSound;
	char* Flags;
	char* RaiseState;
	uint32_t RaiseStateID;
	char* FlagsTwo;
		}
	}
	
	/* Cleanup */
	fprintf(stderr, "done!\n");
	
	/* Dump REMOODAT */
	// Objects
	for (i = 0; i < l_NumInfos; i++)
	{
		// Get Current
		CurInfo = &l_Infos[i];
		
		// Header
		fprintf(stdout, "// Object -- %s\n", CurInfo->ClassName);
		fprintf(stdout, "MapObject \"%s\"\n", CurInfo->ClassName);
		fprintf(stdout, "{\n");
		
		// Fields
		fprintf(stdout, "\tNiceName \"%s\";\n", CurInfo->ClassName);
		fprintf(stdout, "\tHereticEdNum \"%i\";\n", CurInfo->DoomEdNum);
		fprintf(stdout, "\tDeHackEdNum \"%i\";\n", CurInfo->DEHId);
		fprintf(stdout, "\tMTName \"%s\";\n", CurInfo->MTName);
		fprintf(stdout, "\tSpawnHealth \"%i\";\n", CurInfo->SpawnHealth);
		
		if (CurInfo->Damage)
			fprintf(stdout, "\tDamage \"%i\";\n", CurInfo->Damage);
		
		fprintf(stdout, "\tMass \"%i\";\n", CurInfo->Mass);
		fprintf(stdout, "\tSpeed \"%i\";\n", CurInfo->Speed);
		fprintf(stdout, "\tPainChance \"%g\";\n", (double)CurInfo->PainChance / 255.0);
		fprintf(stdout, "\tHeight \"%g\";\n", (double)CurInfo->Height);
		fprintf(stdout, "\tRadius \"%g\";\n", (double)CurInfo->Radius);
		
		// Footer
		fprintf(stdout, "}\n\n");
	}
	
	/* Done */
	return EXIT_SUCCESS;
#undef BUFSIZE
}

