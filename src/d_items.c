// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2008-2012 GhostlyDeath (ghostlydeath@gmail.com)
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: holds the weapon info for now...

#include <stdio.h>

// We are referring to sprite numbers.
#include "info.h"
#include "d_items.h"
#include "v_video.h"
#include "p_pspr.h"

void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();
void A_FireShotgun2();
void A_CheckReload();
void A_OpenShotgun2();
void A_LoadShotgun2();
void A_CloseShotgun2();
void A_FireCGun();
void A_GunFlash();
void A_FireMissile();
void A_Saw();
void A_FirePlasma();
void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();
void A_Explode();
void A_Pain();
void A_PlayerScream();
void A_Fall();
void A_XScream();
void A_Look();
void A_Chase();
void A_FaceTarget();
void A_PosAttack();
void A_Scream();
void A_SPosAttack();
void A_VileChase();
void A_VileStart();
void A_VileTarget();
void A_VileAttack();
void A_StartFire();
void A_Fire();
void A_FireCrackle();
void A_Tracer();
void A_SkelWhoosh();
void A_SkelFist();
void A_SkelMissile();
void A_FatRaise();
void A_FatAttack1();
void A_FatAttack2();
void A_FatAttack3();
void A_BossDeath();
void A_CPosAttack();
void A_CPosRefire();
void A_TroopAttack();
void A_SargAttack();
void A_HeadAttack();
void A_BruisAttack();
void A_SkullAttack();
void A_Metal();
void A_SpidRefire();
void A_BabyMetal();
void A_BspiAttack();
void A_Hoof();
void A_CyberAttack();
void A_PainAttack();
void A_PainDie();
void A_KeenDie();
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();

void A_SmokeTrailer();
void A_SmokeTrailerRocket();
void A_SmokeTrailerSkull();

//
// PSPRITE ACTIONS for weapons.
// This struct controls the weapon animations.
//
// Each entry is:
//  ammo/amunition type
//  upstate
//  downstate
//  readystate
//  atkstate, i.e. attack/fire/hit frame
//  flashstate, muzzle flash
//

/* D_DumpStates() -- Dump state frame */
void D_DumpStates(FILE* f, int StateID, const char* const StateGroupName, weaponinfo_t* ThisWep)
{
	state_t* CurrentState;
	int32_t TrigID[NUMSTATES];
	int i, Remember, Trans, NextID;
	const char* TransName;
	
	// Check
	if (StateID == S_NULL)
		return;
	
	// Clear
	memset(TrigID, 0, sizeof(TrigID));
	
	// Pre-map state
	Remember = StateID;
	for (i = 1; !TrigID[StateID] && StateID != S_NULL; i++)
	{
		// Get Current state
		CurrentState = &states[StateID];
		
		// Place ID of current state
		TrigID[StateID] = i;
		
		// If tics is -1, end here
		if (CurrentState->tics < 0)
			break;
		
		// Go to next frame
		StateID = CurrentState->nextstate;
		
		// Do NOT go into frame
		if (StateID == ThisWep->upstate || StateID == ThisWep->downstate ||
			StateID == ThisWep->readystate || StateID == ThisWep->atkstate ||
			StateID == ThisWep->holdatkstate || StateID == ThisWep->flashstate)
			break;
	}
	StateID = Remember;
	
	// Opening
	fprintf(f, "\t\n");
	fprintf(f, "\tState \"%s\"\n", StateGroupName);
	fprintf(f, "\t{\n");
	
	// Get current states
	for (i = 1; !(TrigID[StateID] & 0x8000) && StateID != S_NULL; i++)
	{
		// Get Current state
		CurrentState = &states[StateID];
		
		// Place ID of current state
		TrigID[StateID] |= 0x8000;
		
		// Print
		fprintf(f, "\t\tFrame \"%i\"\n", i);
		fprintf(f, "\t\t{\n");
		
		// Print Information
		fprintf(f, "\t\t\tSprite \"%s\";\n", sprnames[CurrentState->sprite]);
		fprintf(f, "\t\t\tFrame \"%i\";\n", CurrentState->frame & FF_FRAMEMASK);
		fprintf(f, "\t\t\tTics \"%i\";\n", CurrentState->tics);
		
		if (CurrentState->RMODFastTics)
			fprintf(f, "\t\t\tFastTics \"%i\";\n", CurrentState->RMODFastTics);
		
		if (CurrentState->frame & FF_FULLBRIGHT)
			fprintf(f, "\t\t\tFullBright \"true\";\n");
		
		// Priority
		TransName = NULL;
		if (CurrentState->Priority == STP_NULL) TransName = "Null";
		else if (CurrentState->Priority == STP_DEFAULT) TransName = "Default";
		else if (CurrentState->Priority == STP_WEAPON) TransName = "Weapons";
		else if (CurrentState->Priority == STP_AMMO) TransName = "Ammo";
		else if (CurrentState->Priority == STP_WEPFLASH) TransName = "WeaponFlash";
		else if (CurrentState->Priority == STP_EFFECTS) TransName = "Effects";
		else if (CurrentState->Priority == STP_MONSTERS) TransName = "Monsters";
		else if (CurrentState->Priority == STP_CORPSES) TransName = "Corpses";
		else if (CurrentState->Priority == STP_PLAYERS) TransName = "Players";
		else if (CurrentState->Priority == STP_HEALTH) TransName = "Health";
		else if (CurrentState->Priority == STP_COOKIES) TransName = "Cookies";
		else if (CurrentState->Priority == STP_MISSIONCRITICAL) TransName = "MissionCritical";
		else if (CurrentState->Priority == STP_POWERUPS) TransName = "Powerups";
		else if (CurrentState->Priority == STP_DECORATIONS) TransName = "Decorations";
		else if (CurrentState->Priority == STP_PROJECTILES) TransName = "Projectiles";
		
		if (TransName)
			fprintf(f, "\t\t\tViewPriority \"%s\";\n", TransName);
		
		// Action
		TransName = NULL;
		if (CurrentState->action.acv == A_Light0) TransName = "Light0";
		else if (CurrentState->action.acv == A_WeaponReady) TransName = "WeaponReady";
		else if (CurrentState->action.acv == A_Lower) TransName = "Lower";
		else if (CurrentState->action.acv == A_Raise) TransName = "Raise";
		else if (CurrentState->action.acv == A_Punch) TransName = "Punch";
		else if (CurrentState->action.acv == A_ReFire) TransName = "ReFire";
		else if (CurrentState->action.acv == A_FirePistol) TransName = "FirePistol";
		else if (CurrentState->action.acv == A_Light1) TransName = "Light1";
		else if (CurrentState->action.acv == A_FireShotgun) TransName = "FireShotgun";
		else if (CurrentState->action.acv == A_Light2) TransName = "Light2";
		else if (CurrentState->action.acv == A_FireShotgun2) TransName = "FireShotgun2";
		else if (CurrentState->action.acv == A_CheckReload) TransName = "CheckReload";
		else if (CurrentState->action.acv == A_OpenShotgun2) TransName = "OpenShotgun2";
		else if (CurrentState->action.acv == A_LoadShotgun2) TransName = "LoadShotgun2";
		else if (CurrentState->action.acv == A_CloseShotgun2) TransName = "CloseShotgun2";
		else if (CurrentState->action.acv == A_FireCGun) TransName = "FireCGun";
		else if (CurrentState->action.acv == A_GunFlash) TransName = "GunFlash";
		else if (CurrentState->action.acv == A_FireMissile) TransName = "FireMissile";
		else if (CurrentState->action.acv == A_Saw) TransName = "Saw";
		else if (CurrentState->action.acv == A_FirePlasma) TransName = "FirePlasma";
		else if (CurrentState->action.acv == A_BFGsound) TransName = "BFGsound";
		else if (CurrentState->action.acv == A_FireBFG) TransName = "FireBFG";
		else if (CurrentState->action.acv == A_BFGSpray) TransName = "BFGSpray";
		else if (CurrentState->action.acv == A_Explode) TransName = "Explode";
		else if (CurrentState->action.acv == A_Pain) TransName = "Pain";
		else if (CurrentState->action.acv == A_PlayerScream) TransName = "PlayerScream";
		else if (CurrentState->action.acv == A_Fall) TransName = "Fall";
		else if (CurrentState->action.acv == A_XScream) TransName = "XScream";
		else if (CurrentState->action.acv == A_Look) TransName = "Look";
		else if (CurrentState->action.acv == A_Chase) TransName = "Chase";
		else if (CurrentState->action.acv == A_FaceTarget) TransName = "FaceTarget";
		else if (CurrentState->action.acv == A_PosAttack) TransName = "PosAttack";
		else if (CurrentState->action.acv == A_Scream) TransName = "Scream";
		else if (CurrentState->action.acv == A_SPosAttack) TransName = "SPosAttack";
		else if (CurrentState->action.acv == A_VileChase) TransName = "VileChase";
		else if (CurrentState->action.acv == A_VileStart) TransName = "VileStart";
		else if (CurrentState->action.acv == A_VileTarget) TransName = "VileTarget";
		else if (CurrentState->action.acv == A_VileAttack) TransName = "VileAttack";
		else if (CurrentState->action.acv == A_StartFire) TransName = "StartFire";
		else if (CurrentState->action.acv == A_Fire) TransName = "Fire";
		else if (CurrentState->action.acv == A_FireCrackle) TransName = "FireCrackle";
		else if (CurrentState->action.acv == A_Tracer) TransName = "Tracer";
		else if (CurrentState->action.acv == A_SkelWhoosh) TransName = "SkelWhoosh";
		else if (CurrentState->action.acv == A_SkelFist) TransName = "SkelFist";
		else if (CurrentState->action.acv == A_SkelMissile) TransName = "SkelMissile";
		else if (CurrentState->action.acv == A_FatRaise) TransName = "FatRaise";
		else if (CurrentState->action.acv == A_FatAttack1) TransName = "FatAttack1";
		else if (CurrentState->action.acv == A_FatAttack2) TransName = "FatAttack2";
		else if (CurrentState->action.acv == A_FatAttack3) TransName = "FatAttack3";
		else if (CurrentState->action.acv == A_BossDeath) TransName = "BossDeath";
		else if (CurrentState->action.acv == A_CPosAttack) TransName = "CPosAttack";
		else if (CurrentState->action.acv == A_CPosRefire) TransName = "CPosRefire";
		else if (CurrentState->action.acv == A_TroopAttack) TransName = "TroopAttack";
		else if (CurrentState->action.acv == A_SargAttack) TransName = "SargAttack";
		else if (CurrentState->action.acv == A_HeadAttack) TransName = "HeadAttack";
		else if (CurrentState->action.acv == A_BruisAttack) TransName = "BruisAttack";
		else if (CurrentState->action.acv == A_SkullAttack) TransName = "SkullAttack";
		else if (CurrentState->action.acv == A_Metal) TransName = "Metal";
		else if (CurrentState->action.acv == A_SpidRefire) TransName = "SpidRefire";
		else if (CurrentState->action.acv == A_BabyMetal) TransName = "BabyMetal";
		else if (CurrentState->action.acv == A_BspiAttack) TransName = "BspiAttack";
		else if (CurrentState->action.acv == A_Hoof) TransName = "Hoof";
		else if (CurrentState->action.acv == A_CyberAttack) TransName = "CyberAttack";
		else if (CurrentState->action.acv == A_PainAttack) TransName = "PainAttack";
		else if (CurrentState->action.acv == A_PainDie) TransName = "PainDie";
		else if (CurrentState->action.acv == A_KeenDie) TransName = "KeenDie";
		else if (CurrentState->action.acv == A_BrainPain) TransName = "BrainPain";
		else if (CurrentState->action.acv == A_BrainScream) TransName = "BrainScream";
		else if (CurrentState->action.acv == A_BrainDie) TransName = "BrainDie";
		else if (CurrentState->action.acv == A_BrainAwake) TransName = "BrainAwake";
		else if (CurrentState->action.acv == A_BrainSpit) TransName = "BrainSpit";
		else if (CurrentState->action.acv == A_SpawnSound) TransName = "SpawnSound";
		else if (CurrentState->action.acv == A_SpawnFly) TransName = "SpawnFly";
		else if (CurrentState->action.acv == A_BrainExplode) TransName = "BrainExplode";
		else if (CurrentState->action.acv == A_SmokeTrailer) TransName = "SmokeTrailer";
		else if (CurrentState->action.acv == A_SmokeTrailerRocket) TransName = "SmokeTrailerRocket";
		else if (CurrentState->action.acv == A_SmokeTrailerSkull) TransName = "SmokeTrailerSkull";

		if (TransName)
			fprintf(f, "\t\t\tFunction \"%s\";\n", TransName);
		
		Trans = (CurrentState->frame & FF_TRANSMASK) >> FF_TRANSSHIFT;
		
		TransName = NULL;
		switch (Trans)
		{
			case VEX_TRANS10:
				TransName = "10";
				break;
			case VEX_TRANS20:
				TransName = "20";
				break;
			case VEX_TRANS30:
				TransName = "30";
				break;
			case VEX_TRANS40:
				TransName = "40";
				break;
			case VEX_TRANS50:
				TransName = "50";
				break;
			case VEX_TRANS60:
				TransName = "60";
				break;
			case VEX_TRANS70:
				TransName = "70";
				break;
			case VEX_TRANS80:
				TransName = "80";
				break;
			case VEX_TRANS90:
				TransName = "90";
				break;
			case VEX_TRANSFULL:
				TransName = "100";
				break;
			case VEX_TRANSFIRE:
				TransName = "Fire";
				break;
			case VEX_TRANSFX1:
				TransName = "Effect1";
				break;
			default:
				break;
		}
		
		if (TransName)
			fprintf(f, "\t\t\tTransparency \"%s\";\n", TransName);
		
		NextID = CurrentState->nextstate;
		if (NextID != S_NULL && (NextID == ThisWep->upstate || NextID == ThisWep->downstate ||
			NextID == ThisWep->readystate || NextID == ThisWep->atkstate ||
			NextID == ThisWep->holdatkstate || NextID == ThisWep->flashstate))
		{
			const char* IDName = NULL;
			
			if (NextID == ThisWep->upstate)
				IDName = "PrimaryBringUpState";
			else if (NextID == ThisWep->downstate)
				IDName = "PrimaryPutDownState";
			else if (NextID == ThisWep->readystate)
				IDName = "PrimaryReadyState";
			else if (NextID == ThisWep->atkstate)
				IDName = "PrimaryFireState";
			else if (NextID == ThisWep->holdatkstate)
				IDName = "PrimaryFireHeldState";
			else if (NextID == ThisWep->flashstate)
				IDName = "PrimaryFlashState";
			
			if (IDName)
				fprintf(f, "\t\t\t\Goto \"%s\";\n", IDName);
		}
		else
			fprintf(f, "\t\t\tNext \"%i\";\n", TrigID[CurrentState->nextstate] & 0x7FFF);
		
		// Close
		fprintf(f, "\t\t}\n");
		
		// As long as next state is there!
		if (CurrentState->nextstate != S_NULL && !(TrigID[CurrentState->nextstate] & 0x8000))
			fprintf(f, "\t\t\n");
		
		// If tics is -1, end here
		if (CurrentState->tics < 0)
			break;
		
		// Go to next frame
		StateID = CurrentState->nextstate;
		
		NextID = StateID;
		if (NextID == ThisWep->upstate || NextID == ThisWep->downstate ||
			NextID == ThisWep->readystate || NextID == ThisWep->atkstate ||
			NextID == ThisWep->holdatkstate || NextID == ThisWep->flashstate)
				break;	// Don't loop away!
	}
	
	// Closing
	fprintf(f, "\t}\n");
	
/*	

typedef struct
{
	spritenum_t sprite;
	int32_t frame;				//faB: we use the upper 16bits for translucency
	//     and other shade effects
	int32_t tics;
	// void       (*action) ();
	actionf_t action;
	statenum_t nextstate;
	
	uint8_t Priority;			// View priority of the state
	
	// GhostlyDeath <March 5, 2012> -- To RMOD Deprecation
	int32_t RMODFastTics;						// Tics when -fast
	int32_t ExtraStateFlags;					// Custom flags
} state_t;

extern state_t states[NUMSTATES];

State "SpawnState"
	{
		Frame "1"
		{
			Sprite "PLAY";
			Next "1";
			Tics "-1";
			DEH "S_PLAY";
			DEHId "149";
		}
	}
	

	
	*/
}



/* D_DumpItemRMOD() -- Dump RMOD for weapons and ammo */
void D_DumpItemRMOD(void)
{
	size_t i;
	FILE* f;
	weaponinfo_t* ThisWep;
	ammoinfo_t* ThisAmmo;
	
	/* Create file */
	f = fopen("rmoddump", "w+b");
	
	/* Dump Ammo */
	for (i = 0; i < NUMAMMO; i++)
	{
		// Get current ammo
		ThisAmmo = &ammoinfo[i];
		
		// Print opener
		fprintf(f, "// Ammo -- %s\n", ThisAmmo->ClassName);
		fprintf(f, "MapAmmo \"%s\"\n", ThisAmmo->ClassName);
		fprintf(f, "{\n");
		
		// Print stuff about ammo
		fprintf(f, "\tClipAmmo \"%i\";\n", ThisAmmo->ClipAmmo);
		fprintf(f, "\tMaxAmmo \"%i\";\n", ThisAmmo->MaxAmmo);
		fprintf(f, "\tStartingAmmo \"%i\";\n", ThisAmmo->StartingAmmo);
		
		// Print closer
		fprintf(f, "}\n");
		fprintf(f, "\n");
	}
	
	/* Dump Weapons */
	for (i = 0; i < NUMWEAPONS; i++)
	{
		// Get the current weapon
		ThisWep = &wpnlev1info[i];
		
		// Print opener
		fprintf(f, "// Weapon -- %s\n", ThisWep->NiceName);
		fprintf(f, "MapWeapon \"%s\"\n", ThisWep->ClassName);
		fprintf(f, "{\n");
		
		// Print stuff about weapon
		if (ThisWep->ammo != am_noammo)
			fprintf(f, "\tAmmo \"%s\";\n", ammoinfo[ThisWep->ammo].ClassName);
		else
			fprintf(f, "\tAmmo \"%s\";\n", "None");
		fprintf(f, "\tNiceName \"%s\";\n", ThisWep->NiceName);
		fprintf(f, "\tSBOGraphic \"%s\";\n", ThisWep->SBOGraphic);
		fprintf(f, "\tSlotNum \"%i\";\n", ThisWep->SlotNum);
		fprintf(f, "\tSwitchOrder \"%i\";\n", ThisWep->SwitchOrder);
		fprintf(f, "\tNoAmmoSwitchOrder \"%i\";\n", ThisWep->NoAmmoOrder);
		fprintf(f, "\tAmmoPerShot \"%i\";\n", ThisWep->ammopershoot);
		fprintf(f, "\tPickupAmmo \"%i\";\n", ThisWep->GetAmmo);
		if (ThisWep->DropWeaponClass)
			fprintf(f, "\tDroppedObject \"%s\";\n", ThisWep->DropWeaponClass);
		fprintf(f, "\tSpriteYOffset \"%g\";\n", (float)ThisWep->PSpriteSY / 65536.0);
		
		if (ThisWep->WeaponFlags & WF_ISDOOM)
			fprintf(f, "\tIsDoom \"true\";\n");
		if (ThisWep->WeaponFlags & WF_ISHERETIC)
			fprintf(f, "\tIsHeretic \"true\";\n");
		if (ThisWep->WeaponFlags & WF_ISHEXEN)
			fprintf(f, "\tIsHexen \"true\";\n");
		if (ThisWep->WeaponFlags & WF_ISSTRIFE)
			fprintf(f, "\tIsStrife \"true\";\n");
		if (ThisWep->WeaponFlags & WF_NOTSHAREWARE)
			fprintf(f, "\tIsNotShareware \"true\";\n");
		if (ThisWep->WeaponFlags & WF_INCOMMERCIAL)
			fprintf(f, "\tIsInCommercial \"true\";\n");
		if (ThisWep->WeaponFlags & WF_INREGISTERED)
			fprintf(f, "\tIsRegistered \"true\";\n");
		if (ThisWep->WeaponFlags & WF_BERSERKTOGGLE)
			fprintf(f, "\tIsBerserkToggle \"true\";\n");
		if (ThisWep->WeaponFlags & WF_SWITCHFROMNOAMMO)
			fprintf(f, "\tIsSwitchFromNoAmmo \"true\";\n");
		if (ThisWep->WeaponFlags & WF_STARTINGWEAPON)
			fprintf(f, "\tIsStartingWeapon \"true\";\n");
		
		// Print weapon states
		D_DumpStates(f, ThisWep->upstate, "PrimaryBringUpState", ThisWep);
		D_DumpStates(f, ThisWep->downstate, "PrimaryPutDownState", ThisWep);
		D_DumpStates(f, ThisWep->readystate, "PrimaryReadyState", ThisWep);
		D_DumpStates(f, ThisWep->atkstate, "PrimaryFireState", ThisWep);
		D_DumpStates(f, ThisWep->holdatkstate, "PrimaryFireHeldState", ThisWep);
		D_DumpStates(f, ThisWep->flashstate, "PrimaryFlashState", ThisWep);
		
		// Print closure
		fprintf(f, "}\n");
		fprintf(f, "\n");
	}
	
	/* Close file */
	fclose(f);
}

