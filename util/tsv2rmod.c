#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define TOKSYMS " \t\r\n"

typedef enum
{
	false,
	true
} bool_t;

/* StateInfo_t -- Single State */
typedef struct StateInfo_s
{
	int32_t GroupID;							// State Group ID
	int32_t PassID;								// For Repetition Pass
	
	uint32_t DEHId;
	char* DEHName;
	char* Sprite;
	char* NextState;
	uint32_t NextID;
	char* Func;
	int32_t Frame;
	int32_t Tics;
	bool_t FullBright;
	
	struct StateInfo_s* Next;
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

/* WepInfo_t -- Weapon Info */
typedef struct WepInfo_s
{
	char* UpState;
	uint32_t UpStateID;
	char* DownState;
	uint32_t DownStateID;
	char* ReadyState;
	uint32_t ReadyStateID;
	char* AttackState;
	uint32_t AttackStateID;
	char* HoldState;
	uint32_t HoldStateID;
	char* FlashState;
	uint32_t FlashStateID;
	char* SecUpState;
	uint32_t SecUpStateID;
	char* SecDownState;
	uint32_t SecDownStateID;
	char* SecReadyState;
	uint32_t SecReadyStateID;
	char* SecAttackState;
	uint32_t SecAttackStateID;
	char* SecHoldState;
	uint32_t SecHoldStateID;
	char* SecFlashState;
	uint32_t SecFlashStateID;
} WepInfo_t;

typedef struct StateReader_s
{
	int32_t ID;									// ID of group
	const char* Name;							// Name of group
	size_t StrOff;								// String offset
	size_t IntOff;								// Integer Offset
} StateReader_t;

// c_StateReaders -- For map objects
static const StateReader_t c_StateReaders[] =
{
	{0, "SpawnState", offsetof(MobjInfo_t, SpawnState), offsetof(MobjInfo_t, SpawnStateID)},
	{1, "ActiveState", offsetof(MobjInfo_t, WakeState), offsetof(MobjInfo_t, WakeStateID)},
	{2, "PainState", offsetof(MobjInfo_t, PainState), offsetof(MobjInfo_t, PainStateID)},
	{3, "MeleeAttackState", offsetof(MobjInfo_t, MeleeState), offsetof(MobjInfo_t, MeleeStateID)},
	{4, "RangedAttackState", offsetof(MobjInfo_t, MissileState), offsetof(MobjInfo_t, MissileStateID)},
	{5, "CrashState", offsetof(MobjInfo_t, CrashState), offsetof(MobjInfo_t, CrashStateID)},
	{6, "DeathState", offsetof(MobjInfo_t, DeathState), offsetof(MobjInfo_t, DeathStateID)},
	{7, "GibState", offsetof(MobjInfo_t, XDeathState), offsetof(MobjInfo_t, XDeathStateID)},
	{8, "RaiseState", offsetof(MobjInfo_t, RaiseState), offsetof(MobjInfo_t, RaiseStateID)},
	
	{-1, NULL, 0, 0}
};

// c_WeaponReaders -- Weapons for Heretic
static const StateReader_t c_WeaponReaders[] =
{
	{0, "UpState", offsetof(WepInfo_t, UpState), offsetof(WepInfo_t, UpStateID)},
	{1, "DownState", offsetof(WepInfo_t, DownState), offsetof(WepInfo_t, DownStateID)},
	{2, "ReadyState", offsetof(WepInfo_t, ReadyState), offsetof(WepInfo_t, ReadyStateID)},
	{3, "AttackState", offsetof(WepInfo_t, AttackState), offsetof(WepInfo_t, AttackStateID)},
	{4, "HoldState", offsetof(WepInfo_t, HoldState), offsetof(WepInfo_t, HoldStateID)},
	{5, "FlashState", offsetof(WepInfo_t, FlashState), offsetof(WepInfo_t, FlashStateID)},
	
	{6, "SecUpState", offsetof(WepInfo_t, SecUpState), offsetof(WepInfo_t, UpStateID)},
	{7, "SecDownState", offsetof(WepInfo_t, SecDownState), offsetof(WepInfo_t, SecDownStateID)},
	{8, "SecReadyState", offsetof(WepInfo_t, SecReadyState), offsetof(WepInfo_t, SecReadyStateID)},
	{9, "SecAttackState", offsetof(WepInfo_t, SecAttackState), offsetof(WepInfo_t, SecAttackStateID)},
	{10, "SecHoldState", offsetof(WepInfo_t, SecHoldState), offsetof(WepInfo_t, SecHoldStateID)},
	{11, "SecFlashState", offsetof(WepInfo_t, SecFlashState), offsetof(WepInfo_t, SecFlashStateID)},
	
	{-1, NULL, 0, 0}
};

static StateInfo_t* l_States;					// State List
static size_t l_NumStates;

static MobjInfo_t* l_Infos;						// Objects
static size_t l_NumInfos;

static WepInfo_t* l_Weps;						// Objects
static size_t l_NumWeps;

/* INFO_FlagInfo_t -- Flag Information */
typedef struct INFO_FlagInfo_s
{
	const char* const Old;
	const char* const New;
} INFO_FlagInfo_t;

// c_xFlags -- "flags"
static const INFO_FlagInfo_t c_FlagNames[] =
{
	{"MF_SPECIAL", "IsSpecial"},
	{"MF_SOLID", "IsSolid"},
	{"MF_SHOOTABLE", "IsShootable"},
	{"MF_NOSECTOR", "NoSectorLinks"},
	{"MF_NOBLOCKMAP", "NoBlockMap"},
	{"MF_AMBUSH", "IsDeaf"},
	{"MF_JUSTHIT", "JustGotHit"},
	{"MF_JUSTATTACKED", "JustAttacked"},
	{"MF_SPAWNCEILING", "SpawnsOnCeiling"},
	{"MF_NOGRAVITY", "NoGravity"},
	{"MF_DROPOFF", "IsDropOff"},
	{"MF_PICKUP", "CanPickupItems"},
	{"MF_NOCLIP", "NoClipping"},
	{"MF_SLIDE", "CanSlideAlongWalls"},
	{"MF_FLOAT", "IsFloating"},
	{"MF_TELEPORT", "NoLineCrossing"},
	{"MF_MISSILE", "IsMissile"},
	{"MF_DROPPED", "IsDropped"},
	{"MF_SHADOW", "IsFuzzyShadow"},
	{"MF_NOBLOOD", "NoBleeding"},
	{"MF_CORPSE", "IsCorpse"},
	{"MF_INFLOAT", "NoFloatAdjust"},
	{"MF_COUNTKILL", "IsKillCountable"},
	{"MF_COUNTITEM", "IsItemCountable"},
	{"MF_SKULLFLY", "IsFlyingSkull"},
	{"MF_NOTDMATCH", "NotInDeathmatch"},
	{"MF_NOCLIPTHING", "NoThingClipping"},
	{"MF2_LOGRAV", "IsLowGravity"},
	{"MF2_WINDTHRUST", "IsWindThrustable"},
	{"MF2_FLOORBOUNCE", "IsFloorBouncer"},
	{"MF2_THRUGHOST", "PassThruGhosts"},
	{"MF2_FLY", "IsFlying"},
	{"MF2_FOOTCLIP", "CanFeetClip"},
	{"MF2_SPAWNFLOAT", "SpawnAtRandomZ"},
	{"MF2_NOTELEPORT", "CannotTeleport"},
	{"MF2_RIP", "MissilesThruSolids"},
	{"MF2_PUSHABLE", "IsPushable"},
	{"MF2_SLIDE", "WallSliding"},
	{"MF2_ONMOBJ", "IsOnObject"},
	{"MF2_PASSMOBJ", "MoveOverUnderObject"},
	{"MF2_CANNOTPUSH", "CannotPushPushables"},
	{"MF2_FEETARECLIPPED", "AreFeetClipped"},
	{"MF2_BOSS", "IsBoss"},
	{"MF2_FIREDAMAGE", "DealsFireDamage"},
	{"MF2_NODMGTHRUST", "NoDamageThrust"},
	{"MF2_TELESTOMP", "CanTeleStomp"},
	{"MF2_FLOATBOB", "FloatBobbing"},
	{"MF2_DONTDRAW", "DoNotDraw"},
	{"MF2_BOUNCES", "CanBounce"},
	{"MF2_FRIENDLY", "IsFriendly"},
	{"MF2_FORCETRANSPARENCY", "ForceTransparency"},
	{"MF2_FLOORHUGGER", "IsFloorHugger"},
	{NULL, NULL},
};

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

/* DumpStates() -- Dump states */
void DumpStates(uintptr_t a_BaseP, const StateReader_t* a_Readers, size_t j)
{
	char** pStateStr;
	uint32_t* pStateID, FrameID;
	bool_t NewLined;
	
	StateInfo_t* SRover;
	
	// Obtain state string and ID
	pStateStr = (void*)((uintptr_t)a_BaseP + a_Readers[j].StrOff);
	pStateID = (void*)((uintptr_t)a_BaseP + a_Readers[j].IntOff);
	
	// Missing?
	if (!*pStateID)
		return;
		
	// Spacing Line
	fprintf(stdout, "\t\n");
	
	// Create marker for it
	fprintf(stdout, "\tState \"%s\"\n", a_Readers[j].Name);
	fprintf(stdout, "\t{\n");
	
	// State Roving Loop
	for (FrameID = 1, SRover = &l_States[*pStateID]; SRover; FrameID++)
	{
		// Clear newlinded ness
		NewLined = false;
		
		// If current frame has no pass ID or group, set it now
		if (SRover->PassID == -1)
		{
			SRover->GroupID = j;
			SRover->PassID = FrameID;
		}
		
		// Print current state landed on
		fprintf(stdout, "\t\tFrame \"%u\"\n", FrameID);
		fprintf(stdout, "\t\t{\n");
		
		// Print details
		fprintf(stdout, "\t\t\tHereHackEdNum \"%u\";\n", SRover->DEHId);
		fprintf(stdout, "\t\t\tOldName \"%s\";\n", SRover->DEHName);
		fprintf(stdout, "\t\t\tSprite \"%s\";\n", SRover->Sprite);
		fprintf(stdout, "\t\t\tFrame \"%u\";\n", SRover->Frame);
		fprintf(stdout, "\t\t\tTics \"%i\";\n", SRover->Tics);
		
		// Fullbright?
		if (SRover->FullBright)
			fprintf(stdout, "\t\t\tFullBright \"true\";\n");
		
		// Not null func
		if (SRover->Func)
			if (strcasecmp(SRover->Func, "NULL"))
				fprintf(stdout, "\t\t\tFunction \"%s\";\n", SRover->Func);
		
		// Next is zero, S_NULL
		if (SRover->NextID == 0)
		{
			fprintf(stdout, "\t\t\tNext \"0\";\n");
			SRover = NULL;	// Bye!
		}
		
		// Otherwise, determine if it is a next or a goto
		else
		{
			// Next has no ID set, claim it as the next frame
			if (SRover->Next->PassID == -1)
			{
				fprintf(stdout, "\t\t\tNext \"%i\";\n", FrameID + 1);
				SRover = SRover->Next;
				NewLined = true;	// For pretty output
			}
			
			// It has an ID set, which means it was claimed
			else
			{
				// If no group (ouch), or same group, use next and break
				if (SRover->Next->GroupID == -1 ||
						SRover->Next->GroupID == j)
				{
					fprintf(stdout, "\t\t\tNext \"%i\";\n", SRover->Next->PassID);
					SRover = NULL;
				}
				
				// If a group was set, then use a goto and break
				else
				{
					// Target frame is not one, use an offset frame
					if (SRover->Next->PassID != 1)
						fprintf(stdout, "\t\t\tGoto \"%s:%i\";\n", a_Readers[SRover->Next->GroupID].Name, SRover->Next->PassID);
					
					// Otherwise mean frame 1
					else
						fprintf(stdout, "\t\t\tGoto \"%s\";\n", a_Readers[SRover->Next->GroupID].Name);
					SRover = NULL;
				}
			}
		}
		
		// End
		fprintf(stdout, "\t\t}\n");
		
		// Newline?
		if (NewLined)
			fprintf(stdout, "\t\t\n");
		
		// Dead?
		if (!SRover)
			break;
	}
	
	// Done
	fprintf(stdout, "\t}\n");
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	char Buf[BUFSIZE], BufB[BUFSIZE];
	char* Tok;
	FILE* inMI, *inST, *inNM;
	
	StateInfo_t* CurState;
	size_t i, j, k;
	size_t CurStateNum, ReadNum;
	
	MobjInfo_t* CurInfo;
	size_t CurInfoNum;
	
	WepInfo_t* WepInfo;
	size_t WepInfoNum;
	
	bool_t NewLined;
	
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
						
						// Full bright?
						if (CurState->Frame >= 32767)
						{
							CurState->FullBright = true;
							CurState->Frame -= 32767;
						}
						break;
						
						// Tics
					case 3:
						CurState->Tics = strtol(Tok, NULL, 0);
						break;
						
						// Function
					case 4:
						CurState->Func = strdup(Tok);
						
						// Skip A_
						if (CurState->Func)
							if (strncasecmp(CurState->Func, "A_", 2) == 0)
								CurState->Func += 2;
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
		l_States[i].Next = &l_States[l_States[i].NextID];
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
		}
	}
	
	/* Cleanup */
	fprintf(stderr, "done!\n");
	
	/* Dump REMOODAT */
	// Objects
	for (i = 0; i < l_NumInfos; i++)
	{
		// Clear thing passes
		for (j = 0; j < l_NumStates; j++)
		{
			l_States[j].GroupID = -1;
			l_States[j].PassID = -1;
		}
		
		// Get Current
		CurInfo = &l_Infos[i];
		
		// Header
		fprintf(stdout, "// Object -- %s\n", CurInfo->ClassName);
		fprintf(stdout, "MapObject \"%s\"\n", CurInfo->ClassName);
		fprintf(stdout, "{\n");
		
		// Fields
		fprintf(stdout, "\tNiceName \"%s\";\n", CurInfo->ClassName);
		fprintf(stdout, "\tHereticEdNum \"%i\";\n", CurInfo->DoomEdNum);
		fprintf(stdout, "\tHereHackEdNum \"%i\";\n", CurInfo->DEHId);
		fprintf(stdout, "\tMTName \"%s\";\n", CurInfo->MTName);
		fprintf(stdout, "\tSpawnHealth \"%i\";\n", CurInfo->SpawnHealth);
		
		if (CurInfo->Damage)
			fprintf(stdout, "\tDamage \"%i\";\n", CurInfo->Damage);
		
		fprintf(stdout, "\tMass \"%i\";\n", CurInfo->Mass);
		fprintf(stdout, "\tSpeed \"%i\";\n", CurInfo->Speed);
		fprintf(stdout, "\tPainChance \"%g\";\n", (double)CurInfo->PainChance / 255.0);
		fprintf(stdout, "\tHeight \"%g\";\n", (double)CurInfo->Height);
		fprintf(stdout, "\tRadius \"%g\";\n", (double)CurInfo->Radius);
		
		if (CurInfo->AttackSound)
			fprintf(stdout,"\tAttackSound \"%s\";\n", CurInfo->AttackSound);
		if (CurInfo->PainSound)
			fprintf(stdout,"\tPainSound \"%s\";\n", CurInfo->PainSound);
		if (CurInfo->DeathSound)
			fprintf(stdout,"\tDeathSound \"%s\";\n", CurInfo->DeathSound);
		if (CurInfo->ActiveSound)
			fprintf(stdout,"\tActiveSound \"%s\";\n", CurInfo->ActiveSound);
		
		// Flags
		NewLined = false;
		for (j = 0; j < 2; j++)
			// Tokenize current set
			for (Tok = strtok((!j ? CurInfo->Flags : CurInfo->FlagsTwo), "| \r\t");
					Tok; Tok = strtok(NULL, "| \r\t"))
				for (k = 0; c_FlagNames[k].Old; k++)
					if (strcasecmp(Tok, c_FlagNames[k].Old) == 0)
					{
						if (!NewLined)
						{
							fprintf(stdout, "\t\n");
							NewLined = true;
						}
						
						fprintf(stdout, "\t%s \"true\";\n", c_FlagNames[k].New);
					}
		
		// State Building Loop
		for (j = 0; j < 9; j++)
		{
			DumpStates((uintptr_t)CurInfo, c_StateReaders, j);
		}
		
		// Footer
		fprintf(stdout, "}\n\n");
	}
	
	/* Run through object tables */
	if (inMI)
		inMI = NULL;
	
	inMI = fopen("here-wep.tsv", "rt");
	
	// Handle
	if (inMI)
	{
		fprintf(stderr, "Parsing weapons");
		
		// Constantly read
		while (!feof(inMI))
		{
			// Read string
			fgets(Buf, BUFSIZE, inMI);
			
			// Print
			fprintf(stderr, ".", Buf);
			
			// Resize
			l_Weps = realloc(l_Weps, sizeof(*l_Weps) * (l_NumWeps + 2));
			WepInfo = &l_Weps[WepInfoNum = l_NumWeps++];
			memset(WepInfo, 0, sizeof(*WepInfo));
			
			// Tokenize
			ReadNum = 0;
			for (Tok = strtok(Buf, TOKSYMS); Tok; Tok = strtok(NULL, TOKSYMS))
				switch (ReadNum++)
				{
#define __QUICKHACK(num,name) case num: WepInfo->name = strdup(Tok); WepInfo->name##ID = StateForName(WepInfo->name); break;

					__QUICKHACK(0, UpState)	
					__QUICKHACK(1, DownState)
					__QUICKHACK(2, ReadyState)
					__QUICKHACK(3, AttackState)
					__QUICKHACK(4, HoldState)
					__QUICKHACK(5, FlashState)
					__QUICKHACK(6, SecUpState)	
					__QUICKHACK(7, SecDownState)
					__QUICKHACK(8, SecReadyState)
					__QUICKHACK(9, SecAttackState)
					__QUICKHACK(10, SecHoldState)
					__QUICKHACK(11, SecFlashState)
					
						// Unknown
					default:
						break;
				}
		}
	}
	
	/* Cleanup */
	fprintf(stderr, "done!\n");
	
	/* Dump REMOODAT */
	// Weapons
	for (i = 0; i < l_NumWeps; i++)
	{
		// Clear thing passes
		for (j = 0; j < l_NumStates; j++)
		{
			l_States[j].GroupID = -1;
			l_States[j].PassID = -1;
		}
		
		// Get Current
		WepInfo = &l_Weps[i];
		
		// Header
		fprintf(stdout, "// Weapon -- %p\n", WepInfo);
		fprintf(stdout, "MapWeapon \"%p\"\n", WepInfo);
		fprintf(stdout, "{\n");
		
		// State Building Loop
		for (j = 0; j < 12; j++)
		{
			DumpStates((uintptr_t)WepInfo, c_WeaponReaders, j);
		}
		
		// Footer
		fprintf(stdout, "}\n\n");
	}
	
	
	/* Done */
	return EXIT_SUCCESS;
#undef BUFSIZE
}

