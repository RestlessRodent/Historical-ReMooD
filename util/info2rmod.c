// Emacs style mode select   -*- C++ -*- 
// -----------------------------------------------------------------------------
// ########   ###### #####   #####  ######   ######  ######
// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##
// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##
// ########   ####   ##    #    ## ##    ## ##    ## ##    ##
// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##
// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##
// ##      ## ###### ##         ##  ######   ######  ######
//                      http://remood.org/
// -----------------------------------------------------------------------------
// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)
// Project Co-Leader: RedZTag                (jostol27@gmail.com)
// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)
// -----------------------------------------------------------------------------
// Copyright (C) 2009-2010 The ReMooD Team.
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: info.c + info.h to ReMooD Map Object Data

/*
	ReMooD Map Object Data
	----------------------------------------------------------------------------
	Specification Document Copyright (C) 2009 GhostlyDeath.
	Specification Document Copyright (C) 2009-2010 The ReMooD Team.
	----------------------------------------------------------------------------
	This format is a textual data format (shown in plain text or unicode). It is
	for the containment of map things to be represented as text instead of C
	structures. It is similar to DECORATE and EDF. It is case insensitive, any
	number with a decimal point is considered a fixed number.
	"//" and "/* ... * /" are considered comments, note that the space between
	the asterisk and the slash should not be there, this is shown as such due
	to that if they were combined together it would end this comment text.
	
	Example of <String> : "Hello"
	Example of <Number> : "200"   -- If a decimal is provided in a number field, it will be truncated.
	Example of <Fixed>  : "2.5"   -- If no decimal is provided, the number will be converted to a fixed.
	Example of <Boolean>: true    -- Either "true" or "false", <Strings> will result in "true",
	                                 <Number> and <Fixed> if non-zero will result in "true.
	                                 
	Please note that fields (such as Editor, Interaction, etc.) do not have to
	be specified, however "DoomEdNum" appearing inside of "Sound" will be
	ignored while in the main scope of MapThing it is valid.
	
	== EXAMPLE =================================================================
	MapThing "Foo"
	{
		Interaction
		{
			DoomEdNum 1337;			// This will be ignored
		}
	}
	
	MapThing "Bar"
	{
		DoomEdNum 1338;				// This will not be ignored
	}
	============================================================================
	
	This is the full layout of all the fields and their subfields. Note that any
	unknown fields will be ignored for future compatibility reasons. However,
	the rule does not apply to fields in the state field.
	
	== LAYOUT ==================================================================
	MapThing <String> <Replaces/Inherits <String>>		// <String> is the object's classname
	{
		// Editor Information -- Loading data from maps
		Editor
		{
			DoomEdNum <Number>;					// Number used for the map object when playing Doom
			HereticEdNum <Number>;				//  "      "    "   "   "   "      "    "      Heretic
			HexenEdNum <Number>;				//  "      "    "   "   "   "      "    "      Hexen
			StrifeEdNum <Number>;				//  "      "    "   "   "   "      "    "      Strife
			EdRadius <Number>;					// Width in pixels of an object
			EdHeight <Number>;					// Height in pixels of an object
		}
		
		// Physics Information
		Interaction
		{
			SpawnHealth <Number>;				// Health object spawns with
			SpawnArmor <Number>;				// Armor object spawns with
			ReactionTime <Number>;				// How long it takes for this object to react
			PainChance <Fixed>;					// Percent chance object will feel pain (0.5 means 50% chance)
			Speed <Number>;						// Speed of an object
			FastSpeed <Number>;					// Speed of an object when fast
			Radius <Fixed>;						// Radius of an object
			Height <Fixed>;						// Height of an object
			Mass <Number>;						// Mass of an object (how easy it gets pushed around)
			Density <Fixed>;					// Density of an object (Sink Slower < 1 (Normal Sink) < Sink Faster)
			Damage <Number>;					// Amount of damage the missile does when it collides into an object
			DamageType <String>;				// Damage type, Either "Normal" or "Fire"
			DeathMessage <String>;				// String to display when object kills the player
			BloodType <String>;					// What blood splats spawn when injured
			MeleeRange <Fixed>;					// Maximum Distance to try a melee attack
			MissileRange <Fixed>;				// Maximum Distance to try a missile attack
			MissileImmunity <String>;			// Missile types the object is immune to
			MissileImmunityByType <String>;		// Object types object is immune to (from their missiles)
			MissileResistant <String>;			// Missile types the object is resistant to
			DropType <String>;					// Item this object drops on death
			DropChance <Fixed>;					// Chance to drop this item
			MissileHeight <Fixed>;				// Missile height above Z Position
			NoTargetType <String>;				// Other types to not target
			NormalDamageFactor <Fixed>;			// % damage to recieve when hit by "Normal" attacks
			FireDamageFactor <Fixed>;			// % damage to recieve when hit by "Fire" attacks
			
			AreFeetClipped <Boolean>;			// Are the sprite's feet clipped?
			AudiblePickupSound <Boolean>;		// Can pickup sound be heard by others?
			CanBounce <Boolean>;				// Can bounce off floors and walls
			CanDropOff <Boolean>;				// Can jump off high cliffs
			CanFeetClip <Boolean>;				// Can feet be clipped?
			CanFloorBounce <Boolean>;			// Can bounce off floor
			CanGather <Boolean>;				// Can gather items
			CanJump <Boolean>;					// Can jump
			CanMinotaurSlam <Boolean>;			// Can do a Minotaur Slam
			CanOpenDoors <Boolean>;				// Monster can open doors
			CanSlide <Boolean>;					// Can slide along walls
			CanSwim <Boolean>;					// Object knows how to swim
			CanTeleportStomp <Boolean>;			// Can stomp others when teleporting
			CanWallBounce <Boolean>;			// Can bounce off walls
			CarryKiller <Boolean>;				// When object kills another object, remember the initial killer
			EnableZCheck <Boolean>;				// Enable Z Collision Checking
			FloatBob <Boolean>;					// Object bobs up and down
			ForceDSparilTeleport <Boolean>;		// Force D'Sparil to teleport when hit by this missile
			IsBoss <Boolean>;					// Object is a major boss
			IsBossCubeSpawnable <Boolean>;		// Romero can spawn this
			IsCorpse <Boolean>;					// Slides down steps
			IsDeaf <Boolean>;					// Monster can't hear anything
			IsDoomItemFog <Boolean>;			// Object is the Doom item respawn fog
			IsDoomPlayer <Boolean>;				// Object is the Doom player
			IsDoomTeleportFog <Boolean>;		// Object is the Doom teleport fog
			IsDropped <Boolean>;				// Dropped by a monster
			IsDSparil <Boolean>;				// Object is D'Sparil
			IsExplosionImmune <Boolean>;		// Object can't be hurt by explosions
			IsFloatable <Boolean>;				// Monster can change it's height at will
			IsFloorHugger <Boolean>;			// Object Z Height is always at the floor
			IsFlying <Boolean>;					// Object is flying
			IsFlyingSkull <Boolean>;			// Lost soul in flight
			IsFriendly <Boolean>;				// Friendly Monster (does not attack players)
			IsGatherable <Boolean>;				// Can be picked up
			IsHereticItemFog <Boolean>;			// Object is the Heretic item respawn fog
			IsHereticPlayer <Boolean>;			// Object is the Heretic player
			IsHereticTeleportFog <Boolean>;		// Object is the Heretic teleport fog
			IsInstantKillImmune <Boolean>;		// Object cannot be instantly killed
			IsItemCountable <Boolean>;			// Counts twords item total (when picked up)
			IsKillCountable <Boolean>;			// Counts twords kill total
			IsLowGravity <Boolean>;				// Alternative gravity
			IsMissile <Boolean>;				// If true, Speed will automatically be multiplied by 65536 and object dies on impact
			IsMissileInstantKill <Boolean>;		// If true, missile on impact instantly kills a target
			IsMonster <Boolean>;				// If true, -nomonsters stops this object from spawning at start
			IsOnGround <Boolean>;				// Object standing on solid floor
			IsOnMobj <Boolean>;					// Object is ontop of another object
			IsPushable <Boolean>;				// Object can be pushed around
			IsShadow <Boolean>;					// Doom Fuzzy Effect
			IsShootable <Boolean>;				// Can be shot
			IsSolid <Boolean>;					// Blocks other things
			IsSwimming <Boolean>;				// Object is swimming
			IsTransparent <Boolean>;			// Heretic Transparency Effect
			IsTouchingWater <Boolean>;			// Object is touching water
			IsUnderWater <Boolean>;				// Object is under water
			IsUnique <Boolean>;					// Only one can appear in the level (on map load)
			IsWindPushable <Boolean>;			// Object can be pushed by the wind
			JustAttacked <Boolean>;				// Object just attacked
			JustHit <Boolean>;					// Object just got hit
			JustHitFloor <Boolean>;				// Object just hit the floor
			KeepSlide <Boolean>;				// Keeps info about sliding along walls
			NoAltDeathmatch <Boolean>;			// Does not appear in alternate deathmatch (by default)
			NoAutoFloat <Boolean>;				// When an enemy approaches, object does not auto height change
			NoBlockMap <Boolean>;				// Does not use block map
			NoBlood <Boolean>;					// Does not bleed, emits puffs instead
			NoChickenMorph <Boolean>;			// Cannot be morphed into a chicken
			NoClip <Boolean>;					// Is not blocked by walls and things
			NoCoop <Boolean>;					// Does not appear in coop (by default)
			NoCTF <Boolean>;					// Does not appear in CTF (by default)
			NoDamageThrust <Boolean>;			// Does not thrust others on collision
			NoDeathmatch <Boolean>;				// Does not appear in deathmatch (by default)
			NoDraw <Boolean>;					// Is not drawn
			NoGravity <Boolean>;				// Object does not feel the force of gravity
			NoHitGhost <Boolean>;				// Missile cannot hit ghost
			NoHitSolid <Boolean>;				// Missile cannot hit solid
			NoMissilesHurtSameType <Boolean>;	// Object's Missile does not harm the same type
			NoLineActivate <Boolean>;			// Does not activate lines
			NoLineClipping <Boolean>;			// Is not blocked by line differences (can walk up/down cliffs)
			NoMoveOverSameType <Boolean>;		// Don't allow movement over the same type
			NoNewDeathmatch <Boolean>;			// Does not appear in new deathmatch (by default)
			NoPushing <Boolean>;				// Object cannot push pushable things
			NoSinglePlayer <Boolean>;			// Does not appear in single player (by default)
			NoSectorLinks <Boolean>;			// Does not use sector links
			NoTarget <Boolean>;					// Other objects cannot target this object
			NoTeleport <Boolean>;				// Can not teleport
			NoThingClipping <Boolean>;			// Is not blocked by things
			NoZChecking <Boolean>;				// Disable Z Collision Checking for walls (similar to NoLineClipping)
			ReducedBossDamage <Boolean>;		// Deal less damage to bosses
			SlowsPlayer <Boolean>;				// Slows the player when hit by this missile
			SpawnAtRandomZ <Boolean>;			// Spawns at random Z Height
			SpawnOnCeiling <Boolean>;			// Spawns on the ceiling instead of the floor
			
			FullBlastWakeSound <Boolean>;		// Wake sound plays at full blast
		}
		
		// Sound Information -- Sounds may be separated by semicolons to indicate multiple sounds
		Sound
		{
			WakeSound <String>;					// Sound to play when the object wakes up
			RoamSound <String>;					// Sound to play when the object roams freely
			AttackSound <String>;				// Sound to play when the object attacks (Melee)
			PainSound <String>;					// Sound to play when the object feels pain
			DeathSound <String>;				// Sound to play when the object dies (does not gib)
			WimpyDeathSound <String>;			// Sound to play when the object dies from < 10% damage (does not gib)
			ExtremeDeathSound <String>;			// Sound to play when the object dies and health falls below -50% (does not gib)
			GibDeathSound <String>;				// Sound to play when the object dies and gibs
			FireDeathSound <String>;			// Sound to play when the object dies from fire damage (does not gib)
			ReviveSound <String>;				// Sound to play when the object is revived by an Arch-Vile
			DoomPickupSound <String>;			// Sound when picked up in Doom
			HereticPickupSound <String>;		// Sound when picked up in Heretic
		}
		
		// Visible Information
		Visible
		{
			TranslationColor <String>;			// Color to replace the green map with
			Colormap <String>;					// Colormap to use to recolor the sprite
		}
		
		// Inventory Information -- Provides may be separated by semicolons to indicate multiple provides
		Inventory
		{
			Provides <String>;					// Powerup to provide (see below)
			PickupString <String>;				// String to show when item is picked up
			PickupStringLowHealth <String>;		// String to show when item is picked up (<25% Health)
			PickupExecute <String>;				// Script to execute when this item is picked up
			KeepInSolo <Boolean>;				// Does this item disappear in solo (once picked up)?
			KeepInCoop <Boolean>;				// Does this item disappear in coop (once picked up)?
			KeepInDeath <Boolean>;				// Does this item disappear in deathmatch (once picked up)?
			KeepInAltDeath <Boolean>;			// Does this item disappear in alt-deathmatch (once picked up)?
			KeepInNewDeath <Boolean>;			// Does this item disappear in new deathmatch (once picked up)?
			KeepInCTF <Boolean>;				// Does this item disappear in CTF (once picked up)?
			Respawns <Boolean>;					// Does this item respawn when respawn items is enabled?
			ValueFactor <Fixed>;				// Manipulates the value of the powerup
			PickupMultiplier <Number>;			// Amount to multiply the pickup value (ex: Receive n powerups instead of 1)
		}
		
		// Compatibility
		Legacy
		{
			MapThingID <String>;				// MT_xxx of the object
			OldClass <String>;					// OldClass of the object
			DeHackEdID <Number>;				// DeHackEd Identification Number
			CastNumber <Number>;				// Position thing appears at in the cast
			CastTitle <String>;					// Title of the thing in cast
		}
		
		// States
		States
		{
			////// Example States
			////// from info.c
			////{Sprite, Frame, Tic, Exec, Next}	// Name
			//{SPR_PUNG, 0, 1, {A_WeaponReady}, S_PUNCH},	// S_PUNCH
			//{SPR_PUNG, 0, 1, {A_Lower}, S_PUNCHDOWN},	// S_PUNCHDOWN
			//{SPR_PUNG, 0, 1, {A_Raise}, S_PUNCHUP},	// S_PUNCHUP
			//{SPR_PUNG, 1, 4, {NULL}, S_PUNCH2},	// S_PUNCH1
			//{SPR_PUNG, 2, 4, {A_Punch}, S_PUNCH3},	// S_PUNCH2
			//{SPR_PUNG, 3, 5, {NULL}, S_PUNCH4},	// S_PUNCH3
			//{SPR_PUNG, 2, 4, {NULL}, S_PUNCH5},	// S_PUNCH4
			//{SPR_PUNG, 1, 5, {A_ReFire}, S_PUNCH},	// S_PUNCH5
			////// RMOD -- mini-fields `Sprite "XXXX":0` as a whole can appear in any order (ex: Exec, Next, Sprite, ...)
			//// Local Ref. Num., Sprite <Sprite>:<Frame>, Exec <Action>, Next <LocalRefNum>:<Tic>, DEH <Name>:<RefNum>
			//// DEH is for DeHackEd, S_PUNCH = 2, so DEH "S_PUNCH":2
			//// For sprites that should be lit (use + after the sprite name such as "PUNG+")
			//// Next state of 0 is S_NULL (Destroy thing)
			//ID 1, Sprite "PUNG":0, Exec "A_WeaponReady", Next 1:1, DEH "S_PUNCH":2;
			//ID 2, Sprite "PUNG":0, Exec "A_Lower", Next 2:1, DEH "S_PUNCHDOWN":3;
			//ID 3, Sprite "PUNG":0, Exec "A_Raise", Next 3:1, DEH "S_PUNCHUP":4;
			//ID 4, Sprite "PUNG":1, Next 5:4, DEH "S_PUNCH1":5;
			//ID 5, Sprite "PUNG":2, Exec "A_Punch", Next 6:4, DEH "S_PUNCH2":6;
			//ID 6, Sprite "PUNG":3, Next 7:5, DEH "S_PUNCH3":7;
			//ID 7, Sprite "PUNG":2, Next 8:4, DEH "S_PUNCH4":8;
			//ID 8, Sprite "PUNG":1, Exec "A_ReFire", Next 1:5, DEH "S_PUNCH5":9;
			
			SpawnState							// States of the object when it spawns
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			ActiveState							// States of the object when it is looking for an enemy
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			PainState							// States of the object when it is in pain
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			MeleeAttackState					// States of the object when it is attacking a close object
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			MissileAttackState					// States of the object when it is attacking a far object
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			DeathState							// State of the object when it dies
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			GibDeathState						// State of the object when it gibs
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			FireDeathState						// State of the object when it burns to death
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			CrashState							// State of the object when it crashing into the ground after death
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			RaiseState							// State of the object when it is resurrected by an Arch-Vile
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
		}
	}
	
	Ammo <String>
	{
		Inventory
		{
			MaxAmmo <Number>;					// Maximum ammo allowed
			ClipSize <Number>;					// Number of ammo inside of a clip
		}
	}
	
	Weapon <String>
	{
		// Physical Interaction
		Interaction
		{
			KillMessage <String>;				// String to display when weapon kills a player
			PuffType <String>;					// Object to spawn when firing (Gunshots)
			DamageMultiplier <Fixed>;			// Amount of damage to multiply
			RayPattern <String>;				// Pattern to use when firing
			RayDamage <String>;					// Damage 
			RayDamageType <String>;				// Damage type of the ray, either "Normal" or "Fire"
			RayCount <Number>;					// Number of rays to emit
			ProjectileThing <String>;			// Projectile to fire when attacking
			ProjectilePattern <String>;			// Pattern to use when firing projectiles
			ProjectileCount <Number>;			// Number of projectiles to fire
			BeserkMultiplier <Number>;			// Amount of damage to multiply when using berserker
			SequenceCount <Number>;				// Number of frames between firing ammo
		}
		
		// Physical Interaction when tomed
		SuperInteraction (if not supplied, normal interaction will be used)
		{
			SuperKillMessage <String>;			// String to display when weapon kills a player when tomed
			SuperPuffType <String>;				// Object to spawn when firing (Gunshots) when tomed
			SuperDamageMultiplier <Fixed>;		// Amount of damage to multiply when tomed
			SuperRayPattern <String>;			// Pattern to use when firing when tomed
			SuperRayDamage <String>;			// Damage  when tomed
			SuperRayDamageType <String>;		// Damage type of the ray, either "Normal" or "Fire" when tomed
			SuperRayCount <Number>;				// Number of rays to emit when tomed
			SuperProjectileThing <String>;		// Projectile to fire when attacking when tomed
			SuperProjectilePattern <String>;	// Pattern to use when firing projectiles when tomed
			SuperProjectileCount <Number>;		// Number of projectiles to fire when tomed
			SuperBeserkMultiplier <Number>;		// Amount of damage to multiply when using berserker when tomed
			SuperSequenceCount <Number>;		// Number of frames between firing ammo when tomed
		}
		
		// Inventory Information
		Inventory
		{
			AmmoType <String>;					// Ammo type to use
			AmmoCount <Number>;					// Amount of this ammo to use
		}
		
		// Inventory Information when tomed (if not supplied, normal inventory will be used)
		SuperInventory
		{
			SuperAmmoType <String>;				// Ammo type to use when tomed
			SuperAmmoCount <Number>;			// Amount of this ammo to use when tomed
		}
		
		// States in normal weapon mode
		States
		{
			UpState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			DownState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			ReadyState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			AttackState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			HeldAttackState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			FlashState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
		}
		
		// States in tomed weapon mode (if not supplied, normal states will be used)
		SuperStates
		{
			SuperUpState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			SuperDownState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			SuperReadyState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			SuperAttackState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			SuperHeldAttackState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
			
			SuperFlashState
			{
				<ID <Number><,<Sprite <String>:<Number>><, Exec <String><, Next <Number>:<Number><, DEH <String>:<Number>>>>>;>
				<...>
			}
		}
	}
	============================================================================
	
	Valid `MapThing::Inventory::Provides` powerups:
		SmallHealth				10% Life
		MediumHealth			25% Life
		LargeHealth				100% Life
		SuperHealth				200% Life
		Armor					= 100% Armor if !(n >= 100)
		SuperArmor				= 200% Armor if !(n >= 200)
		ArmorBonus				1% Armor if !(n >= 200)
		RedCard					a red key card
		RedSkull				a red skull
		BlueCard				a blue key card
		BlueSkull				a blue skull
		YellowCard				a yellow key card
		YellowSkull				a yellow skull
		BlueKey					a blue key (Heretic)
		YellowKey				a yellow key (Heretic)
		GreenKey				a green key (Heretic)
		HealthArtifact
		FlyArtifact
		InvincibilityArtifact
		TomeArtifact
		InvisArtifact
		EggArtifact
		SuperHealthArtifact
		TorchArtifact
		FireBombArtifact
		TeleportArtifact
		InvincibilitySphere
		Beserker
		InvisibilitySphere
		RadiationSuit
		MapRevealer
		LightAmpVisors
		Backpack
		BagOfHolding
		Small<Ammo Class>Ammo
		Large<Ammo Class>Ammo
		<Weapon Class>
		
	Valid `Weapon::Interaction::RayPattern` types
		Perfect
		Fist
		Chainsaw
		Pistol
		Shotgun
		SuperShotgun
		Gauntlets
		SuperGauntlets
		Wand
		SuperWand
		Blaster
	
	Valid `Weapon::Interaction::RayDamage` types
		Fist
		Chainsaw
		Pistol
		Chaingun
		Shotgun
		SuperShotgun
		Staff
		SuperStaff
		Gauntlets
		SuperGauntlets
		Wand
		SuperWand
		Blaster
		
	Valid `Weapon::Interaction::ProjectilePattern` types
		Standard
		Mancubus
		SuperWand
		SuperBlaster
		SuperBlasterNova
		Mace
		CrossbowFX1
		CrossbowFX3
		SuperCrossbowFX2
		SuperCrossbowFX3
		SkullRod
		SuperSkullRod
		PhoenixRod
		SuperPhoenixRod
	
	== EXAMPLE FILE ============================================================
	
	============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "info.h"
#include "p_mobj.h"
#include "sounds.h"

// The defined weapons,
//  including a marker indicating
//  user has not changed weapon.
typedef enum
{
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_chainsaw,
	wp_supershotgun,
	
	// Heretic Weapons
	wp_staff,
	wp_goldwand,
	wp_crossbow,
	wp_blaster,
	wp_skullrod,
	wp_phoenixrod,
	wp_mace,
	wp_gauntlets,
	wp_beak,

	NUMWEAPONS,

	// No pending weapon change.
	wp_nochange
} weapontype_t;

// Ammunition types defined.
typedef enum
{
	am_clip,					// Pistol / chaingun ammo.
	am_shell,					// Shotgun / double barreled shotgun.
	am_cell,					// Plasma rifle, BFG.
	am_misl,					// Missile launcher.
	
	// Heretic Ammo
	am_goldwand,
	am_crossbow,
	am_blaster,
	am_skullrod,
	am_phoenixrod,
	am_mace,

	NUMAMMO,
	am_noammo					// Unlimited for chainsaw / fist.
} ammotype_t;

int maxammo[NUMAMMO] = { 200, 50, 300, 50, 100, 50, 200, 200, 20, 150 };
int clipammo[NUMAMMO] = { 10, 4, 20, 1, 5, 2, 6, 10, 1, 10 };

typedef struct
{
	ammotype_t ammo;
	int ammopershoot;
	int upstate;
	int downstate;
	int readystate;
	int atkstate;
	int holdatkstate;
	int flashstate;

} weaponinfo_t;

extern weaponinfo_t wpnlev1info[NUMWEAPONS];
extern weaponinfo_t wpnlev2info[NUMWEAPONS];

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

weaponinfo_t wpnlev1info[NUMWEAPONS] =
{
	{
	 // fist
	 am_noammo,
	 0,
	 S_PUNCHUP,
	 S_PUNCHDOWN,
	 S_PUNCH,
	 S_PUNCH1,
	 S_PUNCH1,
	 S_NULL}
	,
	{
	 // pistol
	 am_clip,
	 1,
	 S_PISTOLUP,
	 S_PISTOLDOWN,
	 S_PISTOL,
	 S_PISTOL1,
	 S_PISTOL1,
	 S_PISTOLFLASH}
	,
	{
	 // shotgun
	 am_shell,
	 1,
	 S_SGUNUP,
	 S_SGUNDOWN,
	 S_SGUN,
	 S_SGUN1,
	 S_SGUN1,
	 S_SGUNFLASH1}
	,
	{
	 // chaingun
	 am_clip,
	 1,
	 S_CHAINUP,
	 S_CHAINDOWN,
	 S_CHAIN,
	 S_CHAIN1,
	 S_CHAIN1,
	 S_CHAINFLASH1}
	,
	{
	 // missile launcher
	 am_misl,
	 1,
	 S_MISSILEUP,
	 S_MISSILEDOWN,
	 S_MISSILE,
	 S_MISSILE1,
	 S_MISSILE1,
	 S_MISSILEFLASH1}
	,
	{
	 // plasma rifle
	 am_cell,
	 1,
	 S_PLASMAUP,
	 S_PLASMADOWN,
	 S_PLASMA,
	 S_PLASMA1,
	 S_PLASMA1,
	 S_PLASMAFLASH1}
	,
	{
	 // bfg 9000
	 am_cell,
	 40,
	 S_BFGUP,
	 S_BFGDOWN,
	 S_BFG,
	 S_BFG1,
	 S_BFG1,
	 S_BFGFLASH1}
	,
	{
	 // chainsaw
	 am_noammo,
	 0,
	 S_SAWUP,
	 S_SAWDOWN,
	 S_SAW,
	 S_SAW1,
	 S_SAW1,
	 S_NULL}
	,
	{
	 // super shotgun
	 am_shell,
	 2,
	 S_DSGUNUP,
	 S_DSGUNDOWN,
	 S_DSGUN,
	 S_DSGUN1,
	 S_DSGUN1,
	 S_DSGUNFLASH1},
	{							// Staff
	 am_noammo,					// ammo
	 0,
	 S_STAFFUP,					// upstate
	 S_STAFFDOWN,				// downstate
	 S_STAFFREADY,				// readystate
	 S_STAFFATK1_1,				// atkstate
	 S_STAFFATK1_1,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Gold wand
	 am_goldwand,				// ammo
	 USE_GWND_AMMO_1,
	 S_GOLDWANDUP,				// upstate
	 S_GOLDWANDDOWN,			// downstate
	 S_GOLDWANDREADY,			// readystate
	 S_GOLDWANDATK1_1,			// atkstate
	 S_GOLDWANDATK1_1,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Crossbow
	 am_crossbow,				// ammo
	 USE_CBOW_AMMO_1,
	 S_CRBOWUP,					// upstate
	 S_CRBOWDOWN,				// downstate
	 S_CRBOW1,					// readystate
	 S_CRBOWATK1_1,				// atkstate
	 S_CRBOWATK1_1,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Blaster
	 am_blaster,				// ammo
	 USE_BLSR_AMMO_1,
	 S_BLASTERUP,				// upstate
	 S_BLASTERDOWN,				// downstate
	 S_BLASTERREADY,			// readystate
	 S_BLASTERATK1_1,			// atkstate
	 S_BLASTERATK1_3,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Skull rod
	 am_skullrod,				// ammo
	 USE_SKRD_AMMO_1,
	 S_HORNRODUP,				// upstate
	 S_HORNRODDOWN,				// downstate
	 S_HORNRODREADY,			// readystae
	 S_HORNRODATK1_1,			// atkstate
	 S_HORNRODATK1_1,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Phoenix rod
	 am_phoenixrod,				// ammo
	 USE_PHRD_AMMO_1,
	 S_PHOENIXUP,				// upstate
	 S_PHOENIXDOWN,				// downstate
	 S_PHOENIXREADY,			// readystate
	 S_PHOENIXATK1_1,			// atkstate
	 S_PHOENIXATK1_1,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Mace
	 am_mace,					// ammo
	 USE_MACE_AMMO_1,
	 S_MACEUP,					// upstate
	 S_MACEDOWN,				// downstate
	 S_MACEREADY,				// readystate
	 S_MACEATK1_1,				// atkstate
	 S_MACEATK1_2,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Gauntlets
	 am_noammo,					// ammo
	 0,
	 S_GAUNTLETUP,				// upstate
	 S_GAUNTLETDOWN,			// downstate
	 S_GAUNTLETREADY,			// readystate
	 S_GAUNTLETATK1_1,			// atkstate
	 S_GAUNTLETATK1_3,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Beak
	 am_noammo,					// ammo
	 0,
	 S_BEAKUP,					// upstate
	 S_BEAKDOWN,				// downstate
	 S_BEAKREADY,				// readystate
	 S_BEAKATK1_1,				// atkstate
	 S_BEAKATK1_1,				// holdatkstate
	 S_NULL						// flashstate
	 }
};

weaponinfo_t wpnlev2info[NUMWEAPONS] = {

	{
	 // fist
	 am_noammo,
	 0,
	 S_PUNCHUP,
	 S_PUNCHDOWN,
	 S_PUNCH,
	 S_PUNCH1,
	 S_PUNCH1,
	 S_NULL}
	,
	{
	 // pistol
	 am_clip,
	 1,
	 S_PISTOLUP,
	 S_PISTOLDOWN,
	 S_PISTOL,
	 S_PISTOL1,
	 S_PISTOL1,
	 S_PISTOLFLASH}
	,
	{
	 // shotgun
	 am_shell,
	 1,
	 S_SGUNUP,
	 S_SGUNDOWN,
	 S_SGUN,
	 S_SGUN1,
	 S_SGUN1,
	 S_SGUNFLASH1}
	,
	{
	 // chaingun
	 am_clip,
	 1,
	 S_CHAINUP,
	 S_CHAINDOWN,
	 S_CHAIN,
	 S_CHAIN1,
	 S_CHAIN1,
	 S_CHAINFLASH1}
	,
	{
	 // missile launcher
	 am_misl,
	 1,
	 S_MISSILEUP,
	 S_MISSILEDOWN,
	 S_MISSILE,
	 S_MISSILE1,
	 S_MISSILE1,
	 S_MISSILEFLASH1}
	,
	{
	 // plasma rifle
	 am_cell,
	 1,
	 S_PLASMAUP,
	 S_PLASMADOWN,
	 S_PLASMA,
	 S_PLASMA1,
	 S_PLASMA1,
	 S_PLASMAFLASH1}
	,
	{
	 // bfg 9000
	 am_cell,
	 40,
	 S_BFGUP,
	 S_BFGDOWN,
	 S_BFG,
	 S_BFG1,
	 S_BFG1,
	 S_BFGFLASH1}
	,
	{
	 // chainsaw
	 am_noammo,
	 0,
	 S_SAWUP,
	 S_SAWDOWN,
	 S_SAW,
	 S_SAW1,
	 S_SAW1,
	 S_NULL}
	,
	{
	 // super shotgun
	 am_shell,
	 2,
	 S_DSGUNUP,
	 S_DSGUNDOWN,
	 S_DSGUN,
	 S_DSGUN1,
	 S_DSGUN1,
	 S_DSGUNFLASH1},
	{							// Staff
	 am_noammo,					// ammo
	 0,
	 S_STAFFUP2,				// upstate
	 S_STAFFDOWN2,				// downstate
	 S_STAFFREADY2_1,			// readystate
	 S_STAFFATK2_1,				// atkstate
	 S_STAFFATK2_1,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Gold wand
	 am_goldwand,				// ammo
	 USE_GWND_AMMO_2,
	 S_GOLDWANDUP,				// upstate
	 S_GOLDWANDDOWN,			// downstate
	 S_GOLDWANDREADY,			// readystate
	 S_GOLDWANDATK2_1,			// atkstate
	 S_GOLDWANDATK2_1,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Crossbow
	 am_crossbow,				// ammo
	 USE_CBOW_AMMO_2,
	 S_CRBOWUP,					// upstate
	 S_CRBOWDOWN,				// downstate
	 S_CRBOW1,					// readystate
	 S_CRBOWATK2_1,				// atkstate
	 S_CRBOWATK2_1,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Blaster
	 am_blaster,				// ammo
	 USE_BLSR_AMMO_2,
	 S_BLASTERUP,				// upstate
	 S_BLASTERDOWN,				// downstate
	 S_BLASTERREADY,			// readystate
	 S_BLASTERATK2_1,			// atkstate
	 S_BLASTERATK2_3,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Skull rod
	 am_skullrod,				// ammo
	 USE_SKRD_AMMO_2,
	 S_HORNRODUP,				// upstate
	 S_HORNRODDOWN,				// downstate
	 S_HORNRODREADY,			// readystae
	 S_HORNRODATK2_1,			// atkstate
	 S_HORNRODATK2_1,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Phoenix rod
	 am_phoenixrod,				// ammo
	 USE_PHRD_AMMO_2,
	 S_PHOENIXUP,				// upstate
	 S_PHOENIXDOWN,				// downstate
	 S_PHOENIXREADY,			// readystate
	 S_PHOENIXATK2_1,			// atkstate
	 S_PHOENIXATK2_2,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Mace
	 am_mace,					// ammo
	 USE_MACE_AMMO_2,
	 S_MACEUP,					// upstate
	 S_MACEDOWN,				// downstate
	 S_MACEREADY,				// readystate
	 S_MACEATK2_1,				// atkstate
	 S_MACEATK2_1,				// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Gauntlets
	 am_noammo,					// ammo
	 0,
	 S_GAUNTLETUP2,				// upstate
	 S_GAUNTLETDOWN2,			// downstate
	 S_GAUNTLETREADY2_1,		// readystate
	 S_GAUNTLETATK2_1,			// atkstate
	 S_GAUNTLETATK2_3,			// holdatkstate
	 S_NULL						// flashstate
	 },
	{							// Beak
	 am_noammo,					// ammo
	 0,
	 S_BEAKUP,					// upstate
	 S_BEAKDOWN,				// downstate
	 S_BEAKREADY,				// readystate
	 S_BEAKATK2_1,				// atkstate
	 S_BEAKATK2_1,				// holdatkstate
	 S_NULL						// flashstate
	 }
};

char *Color_Names[16] = {
	"Green",
	"Gray",
	"Brown",
	"Red",
	"LightGray",
	"LightBrown",
	"LightRed",
	"LightBlue",
	"Blue",
	"Yellow",
	"Beige"
};

FILE* OutRMOD = NULL;

int StartDoomNum[4] = {31000, 31000, 31000, 31000};
int StartHereNum[4] = {31250, 31250, 31250, 31250};
int StartHexNum[4] = {31500, 31500, 31500, 31500};
int StartStrifeNum[4] = {31750, 31750, 31750, 31750};

char* ReturnCodePointer(void (*act)())
{
	if (act == A_Light0) return "Light0";
	else if (act == A_WeaponReady) return "WeaponReady";
	else if (act == A_Lower) return "Lower";
	else if (act == A_Raise) return "Raise";
	else if (act == A_Punch) return "Punch";
	else if (act == A_ReFire) return "ReFire";
	else if (act == A_FirePistol) return "FirePistol";
	else if (act == A_Light1) return "Light1";
	else if (act == A_FireShotgun) return "FireShotgun";
	else if (act == A_Light2) return "Light2";
	else if (act == A_FireShotgun2) return "FireShotgun2";
	else if (act == A_CheckReload) return "CheckReload";
	else if (act == A_OpenShotgun2) return "OpenShotgun2";
	else if (act == A_LoadShotgun2) return "LoadShotgun2";
	else if (act == A_CloseShotgun2) return "CloseShotgun2";
	else if (act == A_FireCGun) return "FireCGun";
	else if (act == A_GunFlash) return "GunFlash";
	else if (act == A_FireMissile) return "FireMissile";
	else if (act == A_Saw) return "Saw";
	else if (act == A_FirePlasma) return "FirePlasma";
	else if (act == A_BFGsound) return "BFGsound";
	else if (act == A_FireBFG) return "FireBFG";
	else if (act == A_BFGSpray) return "BFGSpray";
	else if (act == A_Explode) return "Explode";
	else if (act == A_Pain) return "Pain";
	else if (act == A_PlayerScream) return "PlayerScream";
	else if (act == A_Fall) return "Fall";
	else if (act == A_XScream) return "XScream";
	else if (act == A_Look) return "Look";
	else if (act == A_Chase) return "Chase";
	else if (act == A_FaceTarget) return "FaceTarget";
	else if (act == A_PosAttack) return "PosAttack";
	else if (act == A_Scream) return "Scream";
	else if (act == A_SPosAttack) return "SPosAttack";
	else if (act == A_VileChase) return "VileChase";
	else if (act == A_VileStart) return "VileStart";
	else if (act == A_VileTarget) return "VileTarget";
	else if (act == A_VileAttack) return "VileAttack";
	else if (act == A_StartFire) return "StartFire";
	else if (act == A_Fire) return "Fire";
	else if (act == A_FireCrackle) return "FireCrackle";
	else if (act == A_Tracer) return "Tracer";
	else if (act == A_SkelWhoosh) return "SkelWhoosh";
	else if (act == A_SkelFist) return "SkelFist";
	else if (act == A_SkelMissile) return "SkelMissile";
	else if (act == A_FatRaise) return "FatRaise";
	else if (act == A_FatAttack1) return "FatAttack1";
	else if (act == A_FatAttack2) return "FatAttack2";
	else if (act == A_FatAttack3) return "FatAttack3";
	else if (act == A_BossDeath) return "BossDeath";
	else if (act == A_CPosAttack) return "CPosAttack";
	else if (act == A_CPosRefire) return "CPosRefire";
	else if (act == A_TroopAttack) return "TroopAttack";
	else if (act == A_SargAttack) return "SargAttack";
	else if (act == A_HeadAttack) return "HeadAttack";
	else if (act == A_BruisAttack) return "BruisAttack";
	else if (act == A_SkullAttack) return "SkullAttack";
	else if (act == A_Metal) return "Metal";
	else if (act == A_SpidRefire) return "SpidRefire";
	else if (act == A_BabyMetal) return "BabyMetal";
	else if (act == A_BspiAttack) return "BspiAttack";
	else if (act == A_Hoof) return "Hoof";
	else if (act == A_CyberAttack) return "CyberAttack";
	else if (act == A_PainAttack) return "PainAttack";
	else if (act == A_PainDie) return "PainDie";
	else if (act == A_KeenDie) return "KeenDie";
	else if (act == A_BrainPain) return "BrainPain";
	else if (act == A_BrainScream) return "BrainScream";
	else if (act == A_BrainDie) return "BrainDie";
	else if (act == A_BrainAwake) return "BrainAwake";
	else if (act == A_BrainSpit) return "BrainSpit";
	else if (act == A_SpawnSound) return "SpawnSound";
	else if (act == A_SpawnFly) return "SpawnFly";
	else if (act == A_BrainExplode) return "BrainExplode";
	else if (act == A_FreeTargMobj) return "FreeTargMobj";
	else if (act == A_RestoreSpecialThing1) return "RestoreSpecialThing1";
	else if (act == A_RestoreSpecialThing2) return "RestoreSpecialThing2";
	else if (act == A_HideThing) return "HideThing";
	else if (act == A_UnHideThing) return "UnHideThing";
	else if (act == A_RestoreArtifact) return "RestoreArtifact";
	else if (act == A_HScream) return "HScream";
	else if (act == A_PodPain) return "PodPain";
	else if (act == A_RemovePod) return "RemovePod";
	else if (act == A_MakePod) return "MakePod";
	else if (act == A_InitKeyGizmo) return "InitKeyGizmo";
	else if (act == A_VolcanoSet) return "VolcanoSet";
	else if (act == A_VolcanoBlast) return "VolcanoBlast";
	else if (act == A_BeastPuff) return "BeastPuff";
	else if (act == A_VolcBallImpact) return "VolcBallImpact";
	else if (act == A_SpawnTeleGlitter) return "SpawnTeleGlitter";
	else if (act == A_SpawnTeleGlitter2) return "SpawnTeleGlitter2";
	else if (act == A_AccTeleGlitter) return "AccTeleGlitter";
	else if (act == A_Light0) return "Light0";
	else if (act == A_WeaponReady) return "WeaponReady";
	else if (act == A_Lower) return "Lower";
	else if (act == A_Raise) return "Raise";
	else if (act == A_StaffAttackPL1) return "StaffAttackPL1";
	else if (act == A_ReFire) return "ReFire";
	else if (act == A_StaffAttackPL2) return "StaffAttackPL2";
	else if (act == A_BeakReady) return "BeakReady";
	else if (act == A_BeakRaise) return "BeakRaise";
	else if (act == A_BeakAttackPL1) return "BeakAttackPL1";
	else if (act == A_BeakAttackPL2) return "BeakAttackPL2";
	else if (act == A_GauntletAttack) return "GauntletAttack";
	else if (act == A_FireBlasterPL1) return "FireBlasterPL1";
	else if (act == A_FireBlasterPL2) return "FireBlasterPL2";
	else if (act == A_SpawnRippers) return "SpawnRippers";
	else if (act == A_FireMacePL1) return "FireMacePL1";
	else if (act == A_FireMacePL2) return "FireMacePL2";
	else if (act == A_MacePL1Check) return "MacePL1Check";
	else if (act == A_MaceBallImpact) return "MaceBallImpact";
	else if (act == A_MaceBallImpact2) return "MaceBallImpact2";
	else if (act == A_DeathBallImpact) return "DeathBallImpact";
	else if (act == A_FireSkullRodPL1) return "FireSkullRodPL1";
	else if (act == A_FireSkullRodPL2) return "FireSkullRodPL2";
	else if (act == A_SkullRodPL2Seek) return "SkullRodPL2Seek";
	else if (act == A_AddPlayerRain) return "AddPlayerRain";
	else if (act == A_HideInCeiling) return "HideInCeiling";
	else if (act == A_SkullRodStorm) return "SkullRodStorm";
	else if (act == A_RainImpact) return "RainImpact";
	else if (act == A_FireGoldWandPL1) return "FireGoldWandPL1";
	else if (act == A_FireGoldWandPL2) return "FireGoldWandPL2";
	else if (act == A_FirePhoenixPL1) return "FirePhoenixPL1";
	else if (act == A_InitPhoenixPL2) return "InitPhoenixPL2";
	else if (act == A_FirePhoenixPL2) return "FirePhoenixPL2";
	else if (act == A_ShutdownPhoenixPL2) return "ShutdownPhoenixPL2";
	else if (act == A_PhoenixPuff) return "PhoenixPuff";
	else if (act == A_FlameEnd) return "FlameEnd";
	else if (act == A_FloatPuff) return "FloatPuff";
	else if (act == A_FireCrossbowPL1) return "FireCrossbowPL1";
	else if (act == A_FireCrossbowPL2) return "FireCrossbowPL2";
	else if (act == A_BoltSpark) return "BoltSpark";
	else if (act == A_NoBlocking) return "NoBlocking";
	else if (act == A_AddPlayerCorpse) return "AddPlayerCorpse";
	else if (act == A_SkullPop) return "SkullPop";
	else if (act == A_FlameSnd) return "FlameSnd";
	else if (act == A_CheckBurnGone) return "CheckBurnGone";
	else if (act == A_CheckSkullFloor) return "CheckSkullFloor";
	else if (act == A_CheckSkullDone) return "CheckSkullDone";
	else if (act == A_Feathers) return "Feathers";
	else if (act == A_ChicLook) return "ChicLook";
	else if (act == A_ChicChase) return "ChicChase";
	else if (act == A_ChicPain) return "ChicPain";
	else if (act == A_ChicAttack) return "ChicAttack";
	else if (act == A_MummyAttack) return "MummyAttack";
	else if (act == A_MummyAttack2) return "MummyAttack2";
	else if (act == A_MummySoul) return "MummySoul";
	else if (act == A_ContMobjSound) return "ContMobjSound";
	else if (act == A_MummyFX1Seek) return "MummyFX1Seek";
	else if (act == A_BeastAttack) return "BeastAttack";
	else if (act == A_SnakeAttack) return "SnakeAttack";
	else if (act == A_SnakeAttack2) return "SnakeAttack2";
	else if (act == A_HHeadAttack) return "HHeadAttack";
	else if (act == A_HBossDeath) return "HBossDeath";
	else if (act == A_HeadIceImpact) return "HeadIceImpact";
	else if (act == A_HeadFireGrow) return "HeadFireGrow";
	else if (act == A_WhirlwindSeek) return "WhirlwindSeek";
	else if (act == A_ClinkAttack) return "ClinkAttack";
	else if (act == A_WizAtk1) return "WizAtk1";
	else if (act == A_WizAtk2) return "WizAtk2";
	else if (act == A_WizAtk3) return "WizAtk3";
	else if (act == A_GhostOff) return "GhostOff";
	else if (act == A_ImpMeAttack) return "ImpMeAttack";
	else if (act == A_ImpMsAttack) return "ImpMsAttack";
	else if (act == A_ImpMsAttack2) return "ImpMsAttack2";
	else if (act == A_ImpDeath) return "ImpDeath";
	else if (act == A_ImpXDeath1) return "ImpXDeath1";
	else if (act == A_ImpXDeath2) return "ImpXDeath2";
	else if (act == A_ImpExplode) return "ImpExplode";
	else if (act == A_KnightAttack) return "KnightAttack";
	else if (act == A_DripBlood) return "DripBlood";
	else if (act == A_Sor1Chase) return "Sor1Chase";
	else if (act == A_Sor1Pain) return "Sor1Pain";
	else if (act == A_Srcr1Attack) return "Srcr1Attack";
	else if (act == A_SorZap) return "SorZap";
	else if (act == A_SorcererRise) return "SorcererRise";
	else if (act == A_SorRise) return "SorRise";
	else if (act == A_SorSightSnd) return "SorSightSnd";
	else if (act == A_Srcr2Decide) return "Srcr2Decide";
	else if (act == A_Srcr2Attack) return "Srcr2Attack";
	else if (act == A_Sor2DthInit) return "Sor2DthInit";
	else if (act == A_SorDSph) return "SorDSph";
	else if (act == A_Sor2DthLoop) return "Sor2DthLoop";
	else if (act == A_SorDExp) return "SorDExp";
	else if (act == A_SorDBon) return "SorDBon";
	else if (act == A_BlueSpark) return "BlueSpark";
	else if (act == A_GenWizard) return "GenWizard";
	else if (act == A_MinotaurAtk1) return "MinotaurAtk1";
	else if (act == A_MinotaurDecide) return "MinotaurDecide";
	else if (act == A_MinotaurAtk2) return "MinotaurAtk2";
	else if (act == A_MinotaurAtk3) return "MinotaurAtk3";
	else if (act == A_MinotaurCharge) return "MinotaurCharge";
	else if (act == A_MntrFloorFire) return "MntrFloorFire";
	else if (act == A_ESound) return "ESound";
	else
		return "Unknown";
}

int main(int argc, char** argv)
{
	int i, j, k, l, m, statecount, stateseek, actualdeh;
	int stateref[NUMSTATES];
	int stateowner[NUMSTATES];
	int statequeue[NUMSTATES];
	int* statecheck;
	int* tenstatecheck[25];
	int* stateptr;
	char* statecheckname;
	char* pickupcheckname;
	char* doublecheckname;
	char nextnum[24];
	
		static const struct
	{
		mobjtype_t type;
		int speed[2];
	} MonsterMissileInfo[] =
	{
		// doom
		{
			MT_BRUISERSHOT,
			{
		15, 20}},
		{
			MT_HEADSHOT,
			{
		10, 20}},
		{
			MT_TROOPSHOT,
			{
		10, 20}},
		// heretic
		{
			MT_IMPBALL,
			{
		10, 20}},
		{
			MT_MUMMYFX1,
			{
		9, 18}},
		{
			MT_KNIGHTAXE,
			{
		9, 18}},
		{
			MT_REDAXE,
			{
		9, 18}},
		{
			MT_BEASTBALL,
			{
		12, 20}},
		{
			MT_WIZFX1,
			{
		18, 24}},
		{
			MT_SNAKEPRO_A,
			{
		14, 20}},
		{
			MT_SNAKEPRO_B,
			{
		14, 20}},
		{
			MT_HEADFX1,
			{
		13, 20}},
		{
			MT_HEADFX3,
			{
		10, 18}},
		{
			MT_MNTRFX1,
			{
		20, 26}},
		{
			MT_MNTRFX2,
			{
		14, 20}},
		{
			MT_SRCRFX1,
			{
		20, 28}},
		{
			MT_SOR2FX1,
			{
		20, 28}},
		{
			-1,
			{
		-1, -1}}				// Terminator
	};
	
	printf("info.c + info.h to ReMooD Map Object Data\n");
	printf("Copyright (C) 2009 GhostlyDeath.\n");
	printf("Copyright (C) 2009-2010 The ReMooD Team.\n");
	printf("Available under the GNU GPL 2 (or later)!\n");
	
	OutRMOD = fopen("rmod_mti.lmp", "wt");
	
	if (!OutRMOD)
		return -1;
	
	fprintf(OutRMOD, "// -----------------------------------------------------------------------------\r\n");
	fprintf(OutRMOD, "// ########   ###### #####   #####  ######   ######  ######\r\n");
	fprintf(OutRMOD, "// ##     ##  ##     ##  ## ##  ## ##    ## ##    ## ##   ##\r\n");
	fprintf(OutRMOD, "// ##     ##  ##     ##   ###   ## ##    ## ##    ## ##    ##\r\n");
	fprintf(OutRMOD, "// ########   ####   ##    #    ## ##    ## ##    ## ##    ##\r\n");
	fprintf(OutRMOD, "// ##    ##   ##     ##         ## ##    ## ##    ## ##    ##\r\n");
	fprintf(OutRMOD, "// ##     ##  ##     ##         ## ##    ## ##    ## ##   ##\r\n");
	fprintf(OutRMOD, "// ##      ## ###### ##         ##  ######   ######  ######\r\n");
	fprintf(OutRMOD, "//                      http://remood.org/\r\n");
	fprintf(OutRMOD, "// -----------------------------------------------------------------------------\r\n");
	fprintf(OutRMOD, "// Project Leader:    GhostlyDeath           (ghostlydeath@gmail.com)\r\n");
	fprintf(OutRMOD, "// Project Co-Leader: RedZTag                (jostol27@gmail.com)\r\n");
	fprintf(OutRMOD, "// Members:           TetrisMaster512        (tetrismaster512@hotmail.com)\r\n");
	fprintf(OutRMOD, "// -----------------------------------------------------------------------------\r\n");
	fprintf(OutRMOD, "// Copyright (C) 2009-2010 The ReMooD Team.\r\n");
	fprintf(OutRMOD, "// -----------------------------------------------------------------------------\r\n");
	fprintf(OutRMOD, "// ReMooD Map Object Data (Converted: %u using version %s %s)\r\n", time(NULL), __TIME__, __DATE__);
	fprintf(OutRMOD, "\r\n");
	
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "//                                      DOOM THINGS\r\n");
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "\r\n");
	
	for (i = 0; i < NUMMOBJTYPES; i++)
	{
		// nice bar
		if (i == BEGINHERETIC_MT)
		{
			fprintf(OutRMOD, "// =============================================================================\r\n");
			fprintf(OutRMOD, "//                                    HERETIC THINGS\r\n");
			fprintf(OutRMOD, "// =============================================================================\r\n");
			fprintf(OutRMOD, "\r\n");
		}
		
		// Container
		fprintf(OutRMOD, "MapThing \"%s\"\r\n", MT2ReMooDClass[i]);
		fprintf(OutRMOD, "{\r\n");
		
		if (mobjinfo[i].doomednum != -1)
		{
			// Editor
			fprintf(OutRMOD, "\tEditor\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			if (i < BEGINHERETIC_MT)
			{
				fprintf(OutRMOD, "\t\tDoomEdNum %i;\r\n", mobjinfo[i].doomednum);
				fprintf(OutRMOD, "\t\tHereticEdNum %i;\r\n", StartHereNum[0]);
				fprintf(OutRMOD, "\t\tHexenEdNum %i;\r\n", StartHexNum[0]);
				fprintf(OutRMOD, "\t\tStrifeEdNum %i;\r\n", StartStrifeNum[0]);
				StartHereNum[0]++;
				StartHexNum[0]++;
				StartStrifeNum[0]++;
			}
			else
			{
				fprintf(OutRMOD, "\t\tDoomEdNum %i;\r\n", StartDoomNum[1]);
				fprintf(OutRMOD, "\t\tHereticEdNum %i;\r\n", mobjinfo[i].doomednum);
				fprintf(OutRMOD, "\t\tHexenEdNum %i;\r\n", StartHexNum[1]);
				fprintf(OutRMOD, "\t\tStrifeEdNum %i;\r\n", StartStrifeNum[1]);
				StartDoomNum[1]++;
				StartHexNum[1]++;
				StartStrifeNum[1]++;
			}
			
			// Could be used by an editor
			if (mobjinfo[i].radius)
				fprintf(OutRMOD, "\t\tEdRadius %i;\r\n", (int)((float)mobjinfo[i].radius / 65536.0));
			if (mobjinfo[i].height)
				fprintf(OutRMOD, "\t\tEdHeight %i;\r\n", (int)((float)mobjinfo[i].height / 65536.0));

			if (i == MT_PLAYER || i == MT_HPLAYER || i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL))
				fprintf(OutRMOD, "\t\tHasDirection true;\r\n");
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Interaction
		if (mobjinfo[i].spawnhealth || mobjinfo[i].reactiontime || mobjinfo[i].painchance || mobjinfo[i].speed || mobjinfo[i].radius || mobjinfo[i].height || mobjinfo[i].mass || mobjinfo[i].damage || (mobjinfo[i].flags & ~MF_TRANSLATION) || (mobjinfo[i].flags2))
		{
			fprintf(OutRMOD, "\tInteraction\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			if (mobjinfo[i].spawnhealth)
				fprintf(OutRMOD, "\t\tSpawnHealth %i;\r\n", mobjinfo[i].spawnhealth);
			if (mobjinfo[i].reactiontime)
				fprintf(OutRMOD, "\t\tReactionTime %i;\r\n", mobjinfo[i].reactiontime);
			if (mobjinfo[i].painchance)
				fprintf(OutRMOD, "\t\tPainChance %g;\r\n", (float)mobjinfo[i].painchance / 256.0);
			if (mobjinfo[i].speed)
			{
				if (mobjinfo[i].flags & MF_MISSILE)
					fprintf(OutRMOD, "\t\tSpeed %i;\r\n", (int)((float)mobjinfo[i].speed / 65536.0));
				else
					fprintf(OutRMOD, "\t\tSpeed %i;\r\n", mobjinfo[i].speed);
					
				for (j = 0; MonsterMissileInfo[j].type != -1; j++)
					if (i == MonsterMissileInfo[j].type)
					{
						fprintf(OutRMOD, "\t\tFastSpeed %i;\r\n", MonsterMissileInfo[j].speed[1]);
						break;
					}
			}
			
			if (mobjinfo[i].radius)
				fprintf(OutRMOD, "\t\tRadius %g;\r\n", (float)mobjinfo[i].radius / 65536.0);
			
			if (mobjinfo[i].height)
				fprintf(OutRMOD, "\t\tHeight %g;\r\n", (float)mobjinfo[i].height / 65536.0);
				
			if (mobjinfo[i].mass)
				fprintf(OutRMOD, "\t\tMass %i;\r\n", mobjinfo[i].mass);
				
			fprintf(OutRMOD, "\t\tDensity %g;\r\n", 1.0);
				
			if (mobjinfo[i].damage)
				fprintf(OutRMOD, "\t\tDamage %i;\r\n", mobjinfo[i].damage);
			
			if (mobjinfo[i].damage && mobjinfo[i].flags2 & MF2_FIREDAMAGE || i == MT_PHOENIXFX1)
				fprintf(OutRMOD, "\t\tDamageType \"Fire\";\r\n");
			else if (i == MT_EGGFX)
				fprintf(OutRMOD, "\t\tDamageType \"ChickenMorph\";\r\n");
			else if (i == MT_WHIRLWIND)
				fprintf(OutRMOD, "\t\tDamageType \"WhirlWind\";\r\n");
			else if (mobjinfo[i].damage)
				fprintf(OutRMOD, "\t\tDamageType \"Normal\";\r\n");
			
			if (i == MT_HHEAD)
				fprintf(OutRMOD, "\t\tMissileResistant \"%s;%s\";\r\n", MT2ReMooDClass[MT_BLASTERFX1], MT2ReMooDClass[MT_RIPPER]);
			
			if (i == MT_BRUISER)
				fprintf(OutRMOD, "\t\tMissileImmunityByType \"%s\";\r\n", MT2ReMooDClass[MT_KNIGHT]);
			else if (i == MT_KNIGHT)
				fprintf(OutRMOD, "\t\tMissileImmunityByType \"%s\";\r\n", MT2ReMooDClass[MT_BRUISER]);
				
			// Blood?
			if (!(mobjinfo[i].flags & MF_NOBLOOD) && mobjinfo[i].flags & MF_SHOOTABLE)
			{
				if (i < BEGINHERETIC_MT)
					fprintf(OutRMOD, "\t\tBloodType \"DoomBlood\";\r\n");
				else
					fprintf(OutRMOD, "\t\tBloodType \"HereticBlood\";\r\n");
			}
			
			// Drops?
			if (i == MT_WOLFSS || i == MT_POSSESSED || i == MT_SHOTGUY || i == MT_CHAINGUY)
			{
				if (i == MT_WOLFSS || i == MT_POSSESSED)
					fprintf(OutRMOD, "\t\tDropType \"%s\";\r\n", MT2ReMooDClass[MT_CLIP]);
				else if (i == MT_SHOTGUY)
					fprintf(OutRMOD, "\t\tDropType \"%s\";\r\n", MT2ReMooDClass[MT_SHOTGUN]);
				else if (i == MT_CHAINGUY)
					fprintf(OutRMOD, "\t\tDropType \"%s\";\r\n", MT2ReMooDClass[MT_CHAINGUN]);
					
				fprintf(OutRMOD, "\t\tDropChance %g;\r\n", 1.0);
			}
			
			// Heretic Drops?
			if (i == MT_MUMMY || i == MT_MUMMYLEADER || i == MT_MUMMYGHOST || i == MT_MUMMYLEADERGHOST || i == MT_KNIGHT || i == MT_KNIGHTGHOST || i == MT_WIZARD || i == MT_HHEAD || i == MT_BEAST || i == MT_CLINK || i == MT_SNAKE || i == MT_MINOTAUR)
			{
				switch (i)
				{
					case MT_MUMMY:
					case MT_MUMMYLEADER:
					case MT_MUMMYGHOST:
					case MT_MUMMYLEADERGHOST:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMGWNDWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 3);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						break;
					case MT_KNIGHT:
					case MT_KNIGHTGHOST:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMCBOWWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 5);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						break;
					case MT_WIZARD:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMBLSRWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 10);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						fprintf(OutRMOD, "\t\tHereticSecondDropType \"%s\";\r\n", MT2ReMooDClass[MT_ARTITOMEOFPOWER]);
						fprintf(OutRMOD, "\t\tHereticSecondDropChance %g;\r\n", 4.0/256.0);
						break;
					case MT_HHEAD:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMBLSRWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 10);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						fprintf(OutRMOD, "\t\tHereticSecondDropType \"%s\";\r\n", MT2ReMooDClass[MT_ARTIEGG]);
						fprintf(OutRMOD, "\t\tHereticSecondDropChance %g;\r\n", 51.0/256.0);
						break;
					case MT_BEAST:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMCBOWWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 10);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						break;
					case MT_CLINK:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMSKRDWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 20);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						break;
					case MT_SNAKE:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMPHRDWIMPY]);
						fprintf(OutRMOD, "\t\tHereticFirstDropExtra %i;\r\n", 5);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 84.0/256.0);
						break;
					case MT_MINOTAUR:
						fprintf(OutRMOD, "\t\tHereticFirstDropType \"%s\";\r\n", MT2ReMooDClass[MT_ARTISUPERHEAL]);
						fprintf(OutRMOD, "\t\tHereticFirstDropChance %g;\r\n", 51.0/256.0);
						fprintf(OutRMOD, "\t\tHereticSecondDropType \"%s\";\r\n", MT2ReMooDClass[MT_AMPHRDWIMPY]);
						fprintf(OutRMOD, "\t\tHereticSecondDropExtra %i;\r\n", 10);
						fprintf(OutRMOD, "\t\tHereticSecondDropChance %g;\r\n", 51.0/256.0);
						break;
					default:
						break;
				}
			}
			
			// Height missile spawns at
			if (i == MT_MNTRFX1)
				fprintf(OutRMOD, "\t\tMissileHeight %g;\r\n", 40.0);
			else if (i == MT_MNTRFX2)
				fprintf(OutRMOD, "\t\tMissileHeight %g;\r\n", 0.0);
			else if (i == MT_SRCRFX1)
				fprintf(OutRMOD, "\t\tMissileHeight %g;\r\n", 48.0);
			else if (i == MT_KNIGHTAXE || i == MT_REDAXE)
				fprintf(OutRMOD, "\t\tMissileHeight %g;\r\n", 36.0);
			
			// MF Flags
			if (mobjinfo[i].flags2 & MF2_FEETARECLIPPED)
				fprintf(OutRMOD, "\t\tAreFeetClipped true;\r\n");
			if (mobjinfo[i].flags2 & MF2_BOUNCES)
				fprintf(OutRMOD, "\t\tCanBounce true;\r\n");
			if (mobjinfo[i].flags & MF_DROPOFF)
				fprintf(OutRMOD, "\t\tCanDropOff true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FOOTCLIP)
				fprintf(OutRMOD, "\t\tCanFeetClip true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FLOORBOUNCE)
				fprintf(OutRMOD, "\t\tCanFloorBounce true;\r\n");
			if (mobjinfo[i].flags & MF_PICKUP)
				fprintf(OutRMOD, "\t\tCanGather true;\r\n");
			
			if (i == MT_PLAYER || i == MT_HPLAYER)	
				fprintf(OutRMOD, "\t\tCanJump true;\r\n");
			
			if (i == MT_MINOTAUR)
				fprintf(OutRMOD, "\t\tCanMinotaurSlam true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_SLIDE)
				fprintf(OutRMOD, "\t\tCanSlide true;\r\n");
			
			if (i == MT_PLAYER || i == MT_HPLAYER)
				fprintf(OutRMOD, "\t\tCanSwim true;\r\n");
				
			if (mobjinfo[i].flags2 & MF2_TELESTOMP)
				fprintf(OutRMOD, "\t\tCanTeleportStomp true;\r\n");
			
			if (i == MT_BARREL || i == MT_POD)
				fprintf(OutRMOD, "\t\tCarryKiller true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_PASSMOBJ)
				fprintf(OutRMOD, "\t\tEnableZCheck true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FLOATBOB)
				fprintf(OutRMOD, "\t\tFloatBob true;\r\n");
			
			if (i == MT_PHOENIXFX1 || i == MT_HORNRODFX2)
				fprintf(OutRMOD, "\t\tForceDSparilTeleport true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_BOSS)
				fprintf(OutRMOD, "\t\tIsBoss true;\r\n");
			if (mobjinfo[i].flags & MF_CORPSE)
				fprintf(OutRMOD, "\t\tIsCorpse true;\r\n");
			if (mobjinfo[i].flags & MF_AMBUSH)
				fprintf(OutRMOD, "\t\tIsDeaf true;\r\n");
			
			if (i == MT_PLAYER)
				fprintf(OutRMOD, "\t\tIsDoomPlayer true;\r\n");
			if (i == MT_TFOG)
				fprintf(OutRMOD, "\t\tIsDoomTeleportFog true;\r\n");
			if (i == MT_IFOG)
				fprintf(OutRMOD, "\t\tIsDoomItemFog true;\r\n");
			
			if (mobjinfo[i].flags & MF_DROPPED)
				fprintf(OutRMOD, "\t\tIsDropped true;\r\n");
			
			if (i == MT_SORCERER2)
				fprintf(OutRMOD, "\t\tIsDSparil true;\r\n");
			
			if (i == MT_CYBORG || i == MT_SPIDER || i == MT_MINOTAUR || i == MT_SORCERER1 || i == MT_SORCERER2)
				fprintf(OutRMOD, "\t\tIsExplosionImmune true;\r\n");
				
			if (mobjinfo[i].flags & MF_FLOAT)
				fprintf(OutRMOD, "\t\tIsFloatable true;\r\n");
			if (mobjinfo[i].flags & MF_FLOORHUGGER)
				fprintf(OutRMOD, "\t\tIsFloorHugger true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FLY)
				fprintf(OutRMOD, "\t\tIsFlying true;\r\n");
			if (mobjinfo[i].flags & MF_SKULLFLY)
				fprintf(OutRMOD, "\t\tIsFlyingSkull true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FRIENDLY)
				fprintf(OutRMOD, "\t\tIsFriendly true;\r\n");
			if (mobjinfo[i].flags & MF_SPECIAL)
				fprintf(OutRMOD, "\t\tIsGatherable true;\r\n");
			
			if (i == MT_HPLAYER)
				fprintf(OutRMOD, "\t\tIsHereticPlayer true;\r\n");
			if (i == MT_HTFOG)
				fprintf(OutRMOD, "\t\tIsHereticTeleportFog true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_BOSS || i == MT_HHEAD)
				fprintf(OutRMOD, "\t\tIsInstantKillImmune true;\r\n");	
			
			if (mobjinfo[i].flags & MF_COUNTITEM)
				fprintf(OutRMOD, "\t\tIsItemCountable true;\r\n");
			if (mobjinfo[i].flags & MF_COUNTKILL)
				fprintf(OutRMOD, "\t\tIsKillCountable true;\r\n");
			if (mobjinfo[i].flags2 & MF2_LOGRAV)
				fprintf(OutRMOD, "\t\tIsLowGravity true;\r\n");
			if (mobjinfo[i].flags & MF_MISSILE)
				fprintf(OutRMOD, "\t\tIsMissile true;\r\n");
				
			if (i == MT_MACEFX4)
				fprintf(OutRMOD, "\t\tIsMissileInstantKill true;\r\n");
			
			if (i == MT_SKULL || (mobjinfo[i].flags & MF_COUNTKILL))
				fprintf(OutRMOD, "\t\tIsMonster true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_ONMOBJ)
				fprintf(OutRMOD, "\t\tIsOnMobj true;\r\n");
			if (mobjinfo[i].flags2 & MF2_PUSHABLE)
				fprintf(OutRMOD, "\t\tIsPushable true;\r\n");
			if (mobjinfo[i].flags & MF_SHADOW)
				fprintf(OutRMOD, "\t\tIsShadow true;\r\n");
			if (mobjinfo[i].flags & MF_SHOOTABLE)
				fprintf(OutRMOD, "\t\tIsShootable true;\r\n");
			if (mobjinfo[i].flags & MF_SOLID)
				fprintf(OutRMOD, "\t\tIsSolid true;\r\n");
			if (mobjinfo[i].flags2 & MF2_FORCETRANSPARENCY)
				fprintf(OutRMOD, "\t\tIsTransparent true;\r\n");
			
			if (i == MT_WMACE)
				fprintf(OutRMOD, "\t\tIsUnique true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_WINDTHRUST)
				fprintf(OutRMOD, "\t\tIsWindPushable true;\r\n");
			if (mobjinfo[i].flags & MF_JUSTATTACKED)
				fprintf(OutRMOD, "\t\tJustAttacked true;\r\n");
			if (mobjinfo[i].flags & MF_JUSTHIT)
				fprintf(OutRMOD, "\t\tJustHit true;\r\n");
			if (mobjinfo[i].flags & MF_SLIDE)
				fprintf(OutRMOD, "\t\tKeepSlide true;\r\n");
			if (mobjinfo[i].flags & MF_INFLOAT)
				fprintf(OutRMOD, "\t\tNoAutoFloat true;\r\n");
			if (mobjinfo[i].flags & MF_NOBLOCKMAP)
				fprintf(OutRMOD, "\t\tNoBlockMap true;\r\n");
			if (mobjinfo[i].flags & MF_NOBLOOD)
				fprintf(OutRMOD, "\t\tNoBlood true;\r\n");
			
			if (i == MT_POD || i == MT_CHICKEN || i == MT_HHEAD || i == MT_MINOTAUR || i == MT_SORCERER1 || i == MT_SORCERER2)
				fprintf(OutRMOD, "\t\tNoChickenMorph true;\r\n");
			
			if (mobjinfo[i].flags & MF_NOCLIP)
				fprintf(OutRMOD, "\t\tNoClip true;\r\n");
			if (mobjinfo[i].flags2 & MF2_NODMGTHRUST)
				fprintf(OutRMOD, "\t\tNoDamageThrust true;\r\n");
			if (mobjinfo[i].flags & MF_NOTDMATCH)
				fprintf(OutRMOD, "\t\tNoDeathmatch true;\r\n");
			if (mobjinfo[i].flags2 & MF2_DONTDRAW)
				fprintf(OutRMOD, "\t\tNoDraw true;\r\n");
			if (mobjinfo[i].flags & MF_NOGRAVITY)
				fprintf(OutRMOD, "\t\tNoGravity true;\r\n");
			if (mobjinfo[i].flags2 & MF2_THRUGHOST)
				fprintf(OutRMOD, "\t\tNoHitGhost true;\r\n");
			if (mobjinfo[i].flags2 & MF2_RIP)
				fprintf(OutRMOD, "\t\tNoHitSolid true;\r\n");
			
			if (i == MT_ROCKET || i == MT_PLASMA || i == MT_BFG || i == MT_TROOPSHOT || i == MT_HEADSHOT || i == MT_BRUISERSHOT)
				fprintf(OutRMOD, "\t\tNoLineActivate true;\r\n");
			
			if (mobjinfo[i].flags & MF_TELEPORT)
				fprintf(OutRMOD, "\t\tNoLineClipping true;\r\n");
			
			// always true except for players
			if (i != MT_PLAYER && i != MT_HPLAYER)
				fprintf(OutRMOD, "\t\tNoMissilesHurtSameType true;\r\n");
			
			if (i == MT_IMP || i == MT_WIZARD)
				fprintf(OutRMOD, "\t\tNoMoveOverSameType true;\r\n");
				
			if (mobjinfo[i].flags2 & MF2_CANNOTPUSH)
				fprintf(OutRMOD, "\t\tNoPushing true;\r\n");
			if (mobjinfo[i].flags & MF_NOSECTOR)
				fprintf(OutRMOD, "\t\tNoSectorLinks true;\r\n");
			
			if (i == MT_VILE || mobjinfo[i].flags2 & MF2_BOSS)
				fprintf(OutRMOD, "\t\tNoTarget true;\r\n");
			
			if (i == MT_WIZARD)
				fprintf(OutRMOD, "\t\tNoTargetType \"%s\";\r\n", MT2ReMooDClass[i]);
			
			if (mobjinfo[i].flags2 & MF2_NOTELEPORT)
				fprintf(OutRMOD, "\t\tNoTeleport true;\r\n");
			if (mobjinfo[i].flags & MF_NOCLIPTHING)
				fprintf(OutRMOD, "\t\tNoThingClipping true;\r\n");
			
			if (i >= MT_RAINPLR1 && i <= MT_RAINPLR4)
				fprintf(OutRMOD, "\t\tReducedBossDamage	true;\r\n");
				
			if (i == MT_PHOENIXFX2)
				fprintf(OutRMOD, "\t\tSlowsPlayer true;\r\n");
			
			if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
				fprintf(OutRMOD, "\t\tSpawnAtRandomZ true;\r\n");
			if (mobjinfo[i].flags & MF_SPAWNCEILING)
				fprintf(OutRMOD, "\t\tSpawnOnCeiling true;\r\n");
				
			if (i == MT_SPIDER || i == MT_CYBORG || i == MT_SORCERER2 || (mobjinfo[i].flags2 & MF2_BOSS))
				fprintf(OutRMOD, "\t\tFullBlastWakeSound true;\r\n");
			
			if (i == MT_SPIDER || i == MT_CYBORG || i == MT_CHICPLAYER || i == MT_SORCERER1 || i == MT_MINOTAUR)
				fprintf(OutRMOD, "\t\tFullBlastDeathSound true;\r\n");
				
			if (i == MT_VILE)
				fprintf(OutRMOD, "\t\tVileMissileRange true;\r\n");
			if (i == MT_UNDEAD)
				fprintf(OutRMOD, "\t\tRevenantMissileRange true;\r\n");
			if (i == MT_CYBORG || i == MT_SPIDER || i == MT_SKULL)
				fprintf(OutRMOD, "\t\tHalfMissileRange true;\r\n");
			if (i == MT_CYBORG)
				fprintf(OutRMOD, "\t\tCyberdemonMissileRange true;\r\n");
				
			if (i == MT_WIZARD)
				fprintf(OutRMOD, "\t\tRoamSoundIsSeeSound true;\r\n");
				
			if (i == MT_SORCERER2)
				fprintf(OutRMOD, "\t\tFullBlastRoamSound true;\r\n");
				
			if (i == MT_MNTRFX2)
				fprintf(OutRMOD, "\t\tMinotaureExplosion true;\r\n");
			
			if (i == MT_FIREBOMB)
				fprintf(OutRMOD, "\t\tFireBombExplosion true;\r\n");
			
			if (i == MT_SOR2FX1)
				fprintf(OutRMOD, "\t\tDSparilExplosion true;\r\n");
			
			if (i == MT_FATSO)
				fprintf(OutRMOD, "\t\tMapSevenSixSixSix true;\r\n");
				
			if (i == MT_BABY)
				fprintf(OutRMOD, "\t\tMapSevenSixSixSeven true;\r\n");
				
			if (i == MT_KEEN)
				fprintf(OutRMOD, "\t\tCommercialSixSixSix true;\r\n");
			
			if (i == MT_BRUISER)
				fprintf(OutRMOD, "\t\tEpisodeOneSixSixSix true;\r\n");
				
			if (i == MT_CYBORG)
				fprintf(OutRMOD, "\t\tEpisodeTwoExitLevel true;\r\n");
				
			if (i == MT_SPIDER)
				fprintf(OutRMOD, "\t\tEpisodeThreeExitLevel true;\r\n");
				
			if (i == MT_CYBORG)
				fprintf(OutRMOD, "\t\tEpisodeFourASixSixSix true;\r\n");
				
			if (i == MT_SPIDER)
				fprintf(OutRMOD, "\t\tEpisodeFourBSixSixSix true;\r\n");
				
			if (i == MT_BOSSTARGET)
				fprintf(OutRMOD, "\t\tBossBrainTarget true;\r\n");
			
			if (i == MT_HHEAD)
				fprintf(OutRMOD, "\t\tHereticEpisodeOneSpecial true;\r\n");
			if (i == MT_MINOTAUR)
				fprintf(OutRMOD, "\t\tHereticEpisodeTwoSpecial true;\r\n");
			if (i == MT_SORCERER2)
				fprintf(OutRMOD, "\t\tHereticEpisodeThreeSpecial true;\r\n");
			if (i == MT_HHEAD)
				fprintf(OutRMOD, "\t\tHereticEpisodeFourSpecial true;\r\n");
			if (i == MT_MINOTAUR)
				fprintf(OutRMOD, "\t\tHereticEpisodeFiveSpecial true;\r\n");
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Visible
		if (mobjinfo[i].flags & MF_TRANSLATION)
		{
			fprintf(OutRMOD, "\tVisible\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			fprintf(OutRMOD, "\t\tTranslationColor \"%s\"\r\n", Color_Names[(mobjinfo[i].flags & MF_TRANSLATION) >> MF_TRANSSHIFT]);
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Sound
		if ((mobjinfo[i].seesound > 0 && mobjinfo[i].seesound < NUMSFX) ||
			(mobjinfo[i].attacksound > 0 && mobjinfo[i].attacksound < NUMSFX) ||
			(mobjinfo[i].painsound > 0 && mobjinfo[i].painsound < NUMSFX) ||
			(mobjinfo[i].deathsound > 0 && mobjinfo[i].deathsound < NUMSFX) ||
			(mobjinfo[i].activesound > 0 && mobjinfo[i].activesound < NUMSFX) ||
			(i == MT_SOUNDWATERFALL || i == MT_SOUNDWIND))
		{
			fprintf(OutRMOD, "\tSound\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			if (mobjinfo[i].seesound > 0 && mobjinfo[i].seesound < NUMSFX)
				fprintf(OutRMOD, "\t\tWakeSound \"%s%s\";\r\n", (mobjinfo[i].seesound < sfx_gldhit ? "ds" : ""), S_sfx[mobjinfo[i].seesound].name);
				
			if (mobjinfo[i].activesound > 0 && mobjinfo[i].activesound < NUMSFX)
				fprintf(OutRMOD, "\t\tRoamSound \"%s%s\";\r\n", (mobjinfo[i].activesound < sfx_gldhit ? "ds" : ""), S_sfx[mobjinfo[i].activesound].name);
			
			if (mobjinfo[i].attacksound > 0 && mobjinfo[i].attacksound < NUMSFX)
				fprintf(OutRMOD, "\t\tAttackSoundSound \"%s%s\";\r\n", (mobjinfo[i].attacksound < sfx_gldhit ? "ds" : ""), S_sfx[mobjinfo[i].attacksound].name);
			
			if (mobjinfo[i].painsound > 0 && mobjinfo[i].painsound < NUMSFX)
				fprintf(OutRMOD, "\t\tPainSound \"%s%s\";\r\n", (mobjinfo[i].painsound < sfx_gldhit ? "ds" : ""), S_sfx[mobjinfo[i].painsound].name);
			
			if (mobjinfo[i].deathsound > 0 && mobjinfo[i].deathsound < NUMSFX)
				fprintf(OutRMOD, "\t\tDeathSound \"%s%s\";\r\n", (mobjinfo[i].deathsound < sfx_gldhit ? "ds" : ""), S_sfx[mobjinfo[i].deathsound].name);
				
			if (mobjinfo[i].xdeathstate)
			{
				if (i < BEGINHERETIC_MT)
					fprintf(OutRMOD, "\t\tGibDeathSound \"%s%s\";\r\n", (sfx_slop < sfx_gldhit ? "ds" : ""), S_sfx[sfx_slop].name);
				else
					fprintf(OutRMOD, "\t\tGibDeathSound \"%s%s\";\r\n", (sfx_ripslop < sfx_gldhit ? "ds" : ""), S_sfx[sfx_ripslop].name);
			}
			
			if (mobjinfo[i].raisestate)
			{
				if (i < BEGINHERETIC_MT)
					fprintf(OutRMOD, "\t\tReviveSound \"%s%s\";\r\n", (sfx_slop < sfx_gldhit ? "ds" : ""), S_sfx[sfx_slop].name);
				else
					fprintf(OutRMOD, "\t\tReviveSound \"%s%s\";\r\n", (sfx_ripslop < sfx_gldhit ? "ds" : ""), S_sfx[sfx_ripslop].name);
			}
			
			if (i == MT_SOUNDWATERFALL)
				fprintf(OutRMOD, "\t\tEnvironmentSound \"%s%s\";\r\n", (sfx_waterfl < sfx_gldhit ? "ds" : ""), S_sfx[sfx_waterfl].name);
			else if (i == MT_SOUNDWIND)
				fprintf(OutRMOD, "\t\tEnvironmentSound \"%s%s\";\r\n", (sfx_wind < sfx_gldhit ? "ds" : ""), S_sfx[sfx_wind].name);
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		if (mobjinfo[i].spawnstate || mobjinfo[i].seestate || mobjinfo[i].painstate || mobjinfo[i].meleestate || mobjinfo[i].missilestate || mobjinfo[i].crashstate || mobjinfo[i].deathstate || mobjinfo[i].xdeathstate || mobjinfo[i].raisestate || mobjinfo[i].fdeathstate || mobjinfo[i].healstate || mobjinfo[i].brainexplodestate || mobjinfo[i].xcrashstate || mobjinfo[i].risestate || mobjinfo[i].teleportstate || mobjinfo[i].thirdattackstate || mobjinfo[i].fourthattackstate || mobjinfo[i].growstate || mobjinfo[i].keygizmostate || mobjinfo[i].bloodyskullfloorstate || mobjinfo[i].bloodyskulldonestate)
		{
			fprintf(OutRMOD, "\tStates\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			statecheck = NULL;
			
			tenstatecheck[0] = &mobjinfo[i].spawnstate;
			tenstatecheck[1] = &mobjinfo[i].seestate;
			tenstatecheck[2] = &mobjinfo[i].painstate;
			tenstatecheck[3] = &mobjinfo[i].meleestate;
			tenstatecheck[4] = &mobjinfo[i].missilestate;
			tenstatecheck[5] = &mobjinfo[i].crashstate;
			tenstatecheck[6] = &mobjinfo[i].deathstate;
			tenstatecheck[7] = &mobjinfo[i].xdeathstate;
			tenstatecheck[8] = &mobjinfo[i].fdeathstate;
			tenstatecheck[9] = &mobjinfo[i].raisestate;
			tenstatecheck[10] = &mobjinfo[i].healstate;
			tenstatecheck[11] = &mobjinfo[i].brainexplodestate;
			tenstatecheck[12] = &mobjinfo[i].xcrashstate;
			tenstatecheck[13] = &mobjinfo[i].risestate;
			tenstatecheck[14] = &mobjinfo[i].teleportstate;
			tenstatecheck[15] = &mobjinfo[i].thirdattackstate;
			tenstatecheck[16] = &mobjinfo[i].fourthattackstate;
			tenstatecheck[17] = &mobjinfo[i].growstate;
			tenstatecheck[18] = &mobjinfo[i].keygizmostate;
			tenstatecheck[19] = &mobjinfo[i].bloodyskullfloorstate;
			tenstatecheck[20] = &mobjinfo[i].bloodyskulldonestate;
			
			// this is for whitespace control
			l = 0;
			for (k = 0; k < 21; k++)
			{
				switch (k)
				{
					case 0: if (mobjinfo[i].spawnstate) l |= (1 << k); break;
					case 1: if (mobjinfo[i].seestate) l |= (1 << k); break;
					case 2: if (mobjinfo[i].painstate) l |= (1 << k); break;
					case 3: if (mobjinfo[i].meleestate) l |= (1 << k); break;
					case 4: if (mobjinfo[i].missilestate) l |= (1 << k); break;
					case 5: if (mobjinfo[i].crashstate) l |= (1 << k); break;
					case 6: if (mobjinfo[i].deathstate) l |= (1 << k); break;
					case 7: if (mobjinfo[i].xdeathstate) l |= (1 << k); break;
					case 8: if (mobjinfo[i].fdeathstate) l |= (1 << k); break;
					case 9: if (mobjinfo[i].raisestate) l |= (1 << k); break;
					case 10: if (mobjinfo[i].healstate) l |= (1 << k); break;
					case 11: if (mobjinfo[i].brainexplodestate) l |= (1 << k); break;
					case 12: if (mobjinfo[i].xcrashstate) l |= (1 << k); break;
					case 13: if (mobjinfo[i].risestate) l |= (1 << k); break;
					case 14: if (mobjinfo[i].teleportstate) l |= (1 << k); break;
					case 15: if (mobjinfo[i].thirdattackstate) l |= (1 << k); break;
					case 16: if (mobjinfo[i].fourthattackstate) l |= (1 << k); break;
					case 17: if (mobjinfo[i].growstate) l |= (1 << k); break;
					case 18: if (mobjinfo[i].keygizmostate) l |= (1 << k); break;
					case 19: if (mobjinfo[i].bloodyskullfloorstate) l |= (1 << k); break;
					case 20: if (mobjinfo[i].bloodyskulldonestate) l |= (1 << k); break;
					default: continue;
				}
			}
			
			memset(stateref, 0, sizeof(stateref));
			memset(stateowner, 0, sizeof(stateowner));

			for (k = 0; k < 21; k++)
			{
				stateptr = statecheck;
				
				switch (k)
				{
					case 0: statecheck = &mobjinfo[i].spawnstate; statecheckname = "SpawnState"; break;
					case 1: statecheck = &mobjinfo[i].seestate; statecheckname = "ActiveState"; break;
					case 2: statecheck = &mobjinfo[i].painstate; statecheckname = "PainState"; break;
					case 3: statecheck = &mobjinfo[i].meleestate; statecheckname = "MeleeAttackState"; break;
					case 4: statecheck = &mobjinfo[i].missilestate; statecheckname = "MissileAttackState"; break;
					case 5: statecheck = &mobjinfo[i].crashstate; statecheckname = "CrashState"; break;
					case 6: statecheck = &mobjinfo[i].deathstate; statecheckname = "DeathState"; break;
					case 7: statecheck = &mobjinfo[i].xdeathstate; statecheckname = "GibDeathState"; break;
					case 8: statecheck = &mobjinfo[i].fdeathstate; statecheckname = "FireDeathState"; break;
					case 9: statecheck = &mobjinfo[i].raisestate; statecheckname = "RaiseState"; break;
					case 10: statecheck = &mobjinfo[i].healstate; statecheckname = "HealState"; break;
					case 11: statecheck = &mobjinfo[i].brainexplodestate; statecheckname = "BrainExplodeState"; break;
					case 12: statecheck = &mobjinfo[i].xcrashstate; statecheckname = "GibCrashState"; break;
					case 13: statecheck = &mobjinfo[i].risestate; statecheckname = "RiseState"; break;
					case 14: statecheck = &mobjinfo[i].teleportstate; statecheckname = "TeleportState"; break;
					case 15: statecheck = &mobjinfo[i].thirdattackstate; statecheckname = "ThirdAttackState"; break;
					case 16: statecheck = &mobjinfo[i].fourthattackstate; statecheckname = "FourthAttackState"; break;
					case 17: statecheck = &mobjinfo[i].growstate; statecheckname = "GrowState"; break;
					case 18: statecheck = &mobjinfo[i].keygizmostate; statecheckname = "KeyGizmoState"; break;
					case 19: statecheck = &mobjinfo[i].bloodyskullfloorstate; statecheckname = "BloodySkullFloorState"; break;
					case 20: statecheck = &mobjinfo[i].bloodyskulldonestate; statecheckname = "BloodySkullDoneState"; break;
					default: continue;
				}
				
				l &= ~(1 << k);
			
				if (*statecheck)
				{
					fprintf(OutRMOD, "\t\t%s\r\n", statecheckname);
					fprintf(OutRMOD, "\t\t{\r\n");
				
					// Count
					statecount = 0;
					stateseek = *statecheck;
				
					//memset(stateref, 0, sizeof(stateref));
					stateref[0] = 0;

					do 
					{
						statecount++;
						if (stateref[stateseek] == 0)
							stateref[stateseek] = statecount;
						//else
						//	break;
						
						if (stateowner[stateseek] == 0)
							stateowner[stateseek] = k + 1;
						//else
						//	break;
						
						if (stateseek == S_HPLAY_FDTH19 || stateseek == S_SRCR1_ATK3 || stateseek == S_MNTR_ATK3_3 || stateseek == S_HHEADFX3_3)
							stateseek += 1;
						else
							stateseek = states[stateseek].nextstate;
					}
					while (states[stateseek].tics != -1 && 
						stateseek != 0//states[stateseek].nextstate != 0
						&& !stateref[stateseek] && !stateowner[stateseek]);
					
					if (states[stateseek].tics == -1)
					{
						statecount++;
						stateref[stateseek] = statecount;
						stateowner[stateseek] = k + 1;
					}
#if 0					
					do
					{
						statecount++;
						
						if (stateref[stateseek] == 0)
							stateref[stateseek] = statecount;
						
						if (stateowner[stateseek] == 0)
							stateowner[stateseek] = k + 1;
					}
					while (
						states[stateseek].tics != -1 &&
						!stateref[stateseek] &&
						(stateseek = states[stateseek].nextstate) != 0
						);
#endif
					
					// Deploy
					stateseek = *statecheck;
				
					for (j = 0; j < statecount; j++)
					{
						if (stateseek == 0)
							continue;
						
						/* Normal statement */
						m = 0;
						if (stateowner[stateseek] == (k + 1))		// State MAY be a duplicate of another state
						{
							m = 1;
							
							fprintf(OutRMOD, "\t\t\tID %i, Sprite \"%s%s\":%i",
								j + 1,
								sprnames[states[stateseek].sprite],				// Sprite Name
								(states[stateseek].frame & 32768 ? "+" : ""),	// Symbols
								states[stateseek].frame & 32767				// Exclude upper bits
								);
						
							// Next or Next to a Goto?
							if (states[stateseek].tics != -1 && states[stateseek].nextstate != 0 &&
								stateowner[states[stateseek].nextstate] != k + 1)
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									statecount + 1,
									states[stateseek].tics
									);
							else
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									(states[stateseek].nextstate ? stateref[states[stateseek].nextstate] : 0),
									states[stateseek].tics
									);
					
							// Exec
							if (states[stateseek].action.acv)
								fprintf(OutRMOD, ", Exec \"%s\"", ReturnCodePointer(states[stateseek].action.acv));
						
							// Sprite Based Pickup
							if (states[stateseek].sprite == SPR_SHLD || states[stateseek].sprite == SPR_ARM1 ||
								states[stateseek].sprite == SPR_SHD2 || states[stateseek].sprite == SPR_ARM2 ||
								states[stateseek].sprite == SPR_BON1 || states[stateseek].sprite == SPR_BON2 ||
								states[stateseek].sprite == SPR_SOUL || states[stateseek].sprite == SPR_MEGA ||
								states[stateseek].sprite == SPR_BKEY || states[stateseek].sprite == SPR_YKEY ||
								states[stateseek].sprite == SPR_RKEY || states[stateseek].sprite == SPR_BSKU ||
								states[stateseek].sprite == SPR_YSKU || states[stateseek].sprite == SPR_RSKU ||
								states[stateseek].sprite == SPR_BKYY || states[stateseek].sprite == SPR_CKYY ||
								states[stateseek].sprite == SPR_AKYY || states[stateseek].sprite == SPR_PTN1 ||
								states[stateseek].sprite == SPR_STIM || states[stateseek].sprite == SPR_MEDI ||
								states[stateseek].sprite == SPR_PTN2 || states[stateseek].sprite == SPR_SOAR ||
								states[stateseek].sprite == SPR_INVU || states[stateseek].sprite == SPR_PWBK ||
								states[stateseek].sprite == SPR_INVS || states[stateseek].sprite == SPR_EGGC ||
								states[stateseek].sprite == SPR_SPHL || states[stateseek].sprite == SPR_TRCH ||
								states[stateseek].sprite == SPR_FBMB || states[stateseek].sprite == SPR_ATLP ||
								states[stateseek].sprite == SPR_PINV || states[stateseek].sprite == SPR_PSTR ||
								states[stateseek].sprite == SPR_PINS || states[stateseek].sprite == SPR_SUIT ||
								states[stateseek].sprite == SPR_SPMP || states[stateseek].sprite == SPR_PMAP ||
								states[stateseek].sprite == SPR_PVIS || states[stateseek].sprite == SPR_AMG1 ||
								states[stateseek].sprite == SPR_AMG2 || states[stateseek].sprite == SPR_AMM1 ||
								states[stateseek].sprite == SPR_AMM2 || states[stateseek].sprite == SPR_AMC1 ||
								states[stateseek].sprite == SPR_AMC2 || states[stateseek].sprite == SPR_AMB1 ||
								states[stateseek].sprite == SPR_AMB2 || states[stateseek].sprite == SPR_AMS1 ||
								states[stateseek].sprite == SPR_AMS2 || states[stateseek].sprite == SPR_AMP1 ||
								states[stateseek].sprite == SPR_AMP2 || states[stateseek].sprite == SPR_CLIP ||
								states[stateseek].sprite == SPR_AMMO || states[stateseek].sprite == SPR_ROCK ||
								states[stateseek].sprite == SPR_BROK || states[stateseek].sprite == SPR_CELL ||
								states[stateseek].sprite == SPR_CELP || states[stateseek].sprite == SPR_SHEL ||
								states[stateseek].sprite == SPR_SBOX || states[stateseek].sprite == SPR_BPAK ||
								states[stateseek].sprite == SPR_BAGH || states[stateseek].sprite == SPR_BFUG ||
								states[stateseek].sprite == SPR_MGUN || states[stateseek].sprite == SPR_CSAW ||
								states[stateseek].sprite == SPR_LAUN || states[stateseek].sprite == SPR_PLAS ||
								states[stateseek].sprite == SPR_SHOT || states[stateseek].sprite == SPR_SGN2 ||
								states[stateseek].sprite == SPR_WMCE || states[stateseek].sprite == SPR_WBOW ||
								states[stateseek].sprite == SPR_WBLS || states[stateseek].sprite == SPR_WSKL ||
								states[stateseek].sprite == SPR_WPHX || states[stateseek].sprite == SPR_WGNT)
							{
								switch (states[stateseek].sprite)
								{
									case SPR_SHLD: pickupcheckname = "Armor"; break;
									case SPR_ARM1: pickupcheckname = "Armor"; break;
									case SPR_SHD2: pickupcheckname = "SuperArmor"; break;
									case SPR_ARM2: pickupcheckname = "SuperArmor"; break;
									case SPR_BON1: pickupcheckname = "HealthBonus"; break;
									case SPR_BON2: pickupcheckname = "ArmorBonus"; break;
									case SPR_SOUL: pickupcheckname = "LargeHealth"; break;
									case SPR_MEGA: pickupcheckname = "MegaSphere"; break;
									case SPR_BKEY: pickupcheckname = "BlueKey"; break;
									case SPR_YKEY: pickupcheckname = "YellowKey"; break;
									case SPR_RKEY: pickupcheckname = "RedKey"; break;
									case SPR_BSKU: pickupcheckname = "BlueSkull"; break;
									case SPR_YSKU: pickupcheckname = "YellowSkull"; break;
									case SPR_RSKU: pickupcheckname = "RedSkull"; break;
									case SPR_BKYY: pickupcheckname = "BlueKey"; break;
									case SPR_CKYY: pickupcheckname = "YellowKey"; break;
									case SPR_AKYY: pickupcheckname = "GreenKey"; break;
									case SPR_PTN1: pickupcheckname = "SmallHealth"; break;
									case SPR_STIM: pickupcheckname = "SmallHealth"; break;
									case SPR_MEDI: pickupcheckname = "MediumHealth"; break;
									case SPR_PTN2: pickupcheckname = "ArtifactHealing"; break;
									case SPR_SOAR: pickupcheckname = "ArtifactFly"; break;
									case SPR_INVU: pickupcheckname = "ArtifactInvuln"; break;
									case SPR_PWBK: pickupcheckname = "ArtifactTome"; break;
									case SPR_INVS: pickupcheckname = "ArtifactInvis"; break;
									case SPR_EGGC: pickupcheckname = "ArtifactEgg"; break;
									case SPR_SPHL: pickupcheckname = "ArtifactSuperHealth"; break;
									case SPR_TRCH: pickupcheckname = "ArtifactTorch"; break;
									case SPR_FBMB: pickupcheckname = "ArtifactFireBomb"; break;
									case SPR_ATLP: pickupcheckname = "ArtifactTeleport"; break;
									case SPR_PINV: pickupcheckname = "Invincibility"; break;
									case SPR_PSTR: pickupcheckname = "Berserker"; break;
									case SPR_PINS: pickupcheckname = "PartialInvisibility"; break;
									case SPR_SUIT: pickupcheckname = "RadiationSuit"; break;
									case SPR_SPMP: pickupcheckname = "SuperMap"; break;
									case SPR_PMAP: pickupcheckname = "ComputerMap"; break;
									case SPR_PVIS: pickupcheckname = "LightAmpVisor"; break;
									case SPR_AMG1: pickupcheckname = "SmallWandAmmo"; break;
									case SPR_AMG2: pickupcheckname = "LargeWandAmmo"; break;
									case SPR_AMM1: pickupcheckname = "SmallMaceAmmo"; break;
									case SPR_AMM2: pickupcheckname = "LargeMaceAmmo"; break;
									case SPR_AMC1: pickupcheckname = "SmallCrossbowAmmo"; break;
									case SPR_AMC2: pickupcheckname = "LargeCrossbowAmmo"; break;
									case SPR_AMB1: pickupcheckname = "SmallClawAmmo"; break;
									case SPR_AMB2: pickupcheckname = "LargeClawAmmo"; break;
									case SPR_AMS1: pickupcheckname = "SmallSkullRodAmmo"; break;
									case SPR_AMS2: pickupcheckname = "LargeSkullRodAmmo"; break;
									case SPR_AMP1: pickupcheckname = "SmallPhoenixRodAmmo"; break;
									case SPR_AMP2: pickupcheckname = "LargePhoenixRodAmmo"; break;
									case SPR_CLIP: pickupcheckname = "SmallClipAmmo"; break;
									case SPR_AMMO: pickupcheckname = "LargeClipAmmo"; break;
									case SPR_ROCK: pickupcheckname = "SmallRocketAmmo"; break;
									case SPR_BROK: pickupcheckname = "LargeRocketAmmo"; break;
									case SPR_CELL: pickupcheckname = "SmallCellAmmo"; break;
									case SPR_CELP: pickupcheckname = "LargeCellAmmo"; break;
									case SPR_SHEL: pickupcheckname = "SmallShellAmmo"; break;
									case SPR_SBOX: pickupcheckname = "LargeShellAmmo"; break;
									case SPR_BPAK: pickupcheckname = "BackPack"; break;
									case SPR_BAGH: pickupcheckname = "BagOfHolding"; break;
									case SPR_BFUG: pickupcheckname = "BFG9000"; break;
									case SPR_MGUN: pickupcheckname = "Chaingun"; break;
									case SPR_CSAW: pickupcheckname = "Chainsaw"; break;
									case SPR_LAUN: pickupcheckname = "RocketLauncher"; break;
									case SPR_PLAS: pickupcheckname = "PlasmaRifle"; break;
									case SPR_SHOT: pickupcheckname = "Shotgun"; break;
									case SPR_SGN2: pickupcheckname = "SuperShotgun"; break;
									case SPR_WMCE: pickupcheckname = "Mace"; break;
									case SPR_WBOW: pickupcheckname = "Crossbow"; break;
									case SPR_WBLS: pickupcheckname = "DragonClaw"; break;
									case SPR_WSKL: pickupcheckname = "SkullRod"; break;
									case SPR_WPHX: pickupcheckname = "PhoenixRod"; break;
									case SPR_WGNT: pickupcheckname = "Gauntlets"; break;
									default: pickupcheckname = NULL; break;
								}
		
								if (pickupcheckname)
									fprintf(OutRMOD, ", Provides \"%s\"", pickupcheckname);
							}
					
							// DeHackEd
							if (stateseek >= S_FREETARGMOBJ)
							{
								actualdeh = stateseek;
								actualdeh -= S_FREETARGMOBJ;
					
								fprintf(OutRMOD, ", HereticDEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
							else
							{
								actualdeh = stateseek;
								fprintf(OutRMOD, ", DEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
					
							fprintf(OutRMOD, ";\r\n");
						}
					
						if (states[stateseek].tics == -1)
							break;
						
						if (stateseek == S_HPLAY_FDTH19 || stateseek == S_SRCR1_ATK3 || stateseek == S_MNTR_ATK3_3 || stateseek == S_HHEADFX3_3)
							stateseek += 1;
						else
							stateseek = states[stateseek].nextstate;
					
						/* Check For Goto */
						if (stateowner[stateseek] != (k + 1))
							//states[stateseek].tics != -1 && stateowner[stateseek] != (k + 1))
							//states[stateseek].nextstate != 0 &&
							//stateowner[stateseek] != (k + 1))
							//stateowner[states[stateseek].nextstate] != (k + 1))
						{
							switch (stateowner[stateseek] - 1)
							{
								case 0: doublecheckname = "SpawnState"; break;
								case 1: doublecheckname = "ActiveState"; break;
								case 2: doublecheckname = "PainState"; break;
								case 3: doublecheckname = "MeleeAttackState"; break;
								case 4: doublecheckname = "MissileAttackState"; break;
								case 5: doublecheckname = "CrashState"; break;
								case 6: doublecheckname = "DeathState"; break;
								case 7: doublecheckname = "GibDeathState"; break;
								case 8: doublecheckname = "FireDeathState"; break;
								case 9: doublecheckname = "RaiseState"; break;
								case 10: doublecheckname = "HealState"; break;
								case 11: doublecheckname = "BrainExplodeState"; break;
								case 12: doublecheckname = "GibCrashState"; break;
								case 13: doublecheckname = "RiseState"; break;
								case 14: doublecheckname = "TeleportState"; break;
								case 15: doublecheckname = "ThirdAttackState"; break;
								case 16: doublecheckname = "FourthAttackState"; break;
								case 17: doublecheckname = "GrowState"; break;
								case 18: doublecheckname = "KeyGizmoState"; break;
								case 19: doublecheckname = "BloodySkullFloorState"; break;
								case 20: doublecheckname = "BloodySkullDoneState"; break;
								default: doublecheckname = NULL; break;
							}
					
							if (doublecheckname)
							{
								j++;
								
								if (stateref[stateseek] - (m ? 1 : 2))
									snprintf(nextnum, 5, "+%i", stateref[stateseek] - (m ? 1 : 2));
								else
									nextnum[0] = 0;
								
								fprintf(OutRMOD, "\t\t\tID %i, Goto \"%s\"%s;\r\n",
									j + (m ? 1 : 0),
								
									// Goto
									doublecheckname,
									nextnum
									);
								break;
							}
						}
						// ---
					}
				
					fprintf(OutRMOD, "\t\t}\r\n");
					
					if (l)
						fprintf(OutRMOD, "\t\t\r\n");
				}
			}
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Legacy
		fprintf(OutRMOD, "\tLegacy\r\n");
		fprintf(OutRMOD, "\t{\r\n");
		fprintf(OutRMOD, "\t\tMapThingID \"%s\";\r\n", MT2MTString[i]);
		fprintf(OutRMOD, "\t\tOldClass \"%s\";\r\n", MT2ReMooDClass[i]);
		if (i < BEGINHERETIC_MT)
			fprintf(OutRMOD, "\t\tDeHackEdID %i;\r\n", i);
		else
		{
			actualdeh = i;
			actualdeh -= BEGINHERETIC_MT;
			
			fprintf(OutRMOD, "\t\tHereticDeHackEdID %i;\r\n", actualdeh);
		}
		
		// Cast Number
		for (j = 0; castorder[j].type != 0; j++)
			if (i == castorder[j].type)
			{
				fprintf(OutRMOD, "\t\tCastNumber %i;\r\n", j + 1);
				break;
			}
		
		fprintf(OutRMOD, "\t}\r\n");
		
		// Editor
		//fprintf(OutRMOD, "\tEditor\r\n");
		//fprintf(OutRMOD, "\t{\r\n");
		//fprintf(OutRMOD, "\t}\r\n");
		
		fprintf(OutRMOD, "}\r\n");
		fprintf(OutRMOD, "\r\n");
	}
	
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "//                                        AMMO\r\n");
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "\r\n");
	
	for (i = 0; i < NUMAMMO; i++)
	{
		char* AmmoName = NULL;
		
		switch (i)
		{
			case am_clip: AmmoName = "Clip"; break;
			case am_shell: AmmoName = "Shell"; break;
			case am_cell: AmmoName = "Cell"; break;
			case am_misl: AmmoName = "Rocket"; break;
			case am_goldwand: AmmoName = "Wand"; break;
			case am_crossbow: AmmoName = "Crossbow"; break;
			case am_blaster: AmmoName = "Claw"; break;
			case am_skullrod: AmmoName = "SkullRod"; break;
			case am_phoenixrod: AmmoName = "PhoenixRod"; break;
			case am_mace: AmmoName = "Mace"; break;
			default: AmmoName = NULL; break;
		}
		
		if (!AmmoName)
			continue;
			
		fprintf(OutRMOD, "Ammo \"%s\"\r\n", AmmoName);
		fprintf(OutRMOD, "{\r\n");
		
		fprintf(OutRMOD, "\tInventory\r\n");
		fprintf(OutRMOD, "\t{\r\n");
		fprintf(OutRMOD, "\t\tMaximum %i;\r\n", maxammo[i]);
		fprintf(OutRMOD, "\t\tClipSize %i;\r\n", clipammo[i]);
		
		if (i == am_clip)
			fprintf(OutRMOD, "\t\tInitialDoomAmmo 50;\r\n");
		else if (i == am_goldwand)
			fprintf(OutRMOD, "\t\tInitialHereticAmmo 50;\r\n");
		
		fprintf(OutRMOD, "\t}\r\n");
		
		fprintf(OutRMOD, "}\r\n");
		fprintf(OutRMOD, "\r\n");
	}
	
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "//                                       WEAPONS\r\n");
	fprintf(OutRMOD, "// =============================================================================\r\n");
	fprintf(OutRMOD, "\r\n");
	
	for (i = 0; i < NUMWEAPONS; i++)
	{
		char* WeaponName = NULL;
		char* AmmoName = NULL;
		
		switch (wpnlev1info[i].ammo)
		{
			case am_clip: AmmoName = "Clip"; break;
			case am_shell: AmmoName = "Shell"; break;
			case am_cell: AmmoName = "Cell"; break;
			case am_misl: AmmoName = "Rocket"; break;
			case am_goldwand: AmmoName = "Wand"; break;
			case am_crossbow: AmmoName = "Crossbow"; break;
			case am_blaster: AmmoName = "Claw"; break;
			case am_skullrod: AmmoName = "SkullRod"; break;
			case am_phoenixrod: AmmoName = "PhoenixRod"; break;
			case am_mace: AmmoName = "Mace"; break;
			default: AmmoName = NULL; break;
		}
		
		switch (i)
		{
			case wp_fist: WeaponName = "Fist"; break;
			case wp_pistol: WeaponName = "Pistol"; break;
			case wp_shotgun: WeaponName = "Shotgun"; break;
			case wp_chaingun: WeaponName = "Chaingun"; break;
			case wp_missile: WeaponName = "RocketLauncher"; break;
			case wp_plasma: WeaponName = "PlasmaRifle"; break;
			case wp_bfg: WeaponName = "BFG9000"; break;
			case wp_chainsaw: WeaponName = "Chainsaw"; break;
			case wp_supershotgun: WeaponName = "SuperShotgun"; break;
			case wp_staff: WeaponName = "Staff"; break;
			case wp_goldwand: WeaponName = "Wand"; break;
			case wp_crossbow: WeaponName = "Crossbow"; break;
			case wp_blaster: WeaponName = "DragonClaw"; break;
			case wp_skullrod: WeaponName = "SkullRod"; break;
			case wp_phoenixrod: WeaponName = "PhoenixRod"; break;
			case wp_mace: WeaponName = "Mace"; break;
			case wp_gauntlets: WeaponName = "Gauntlets"; break;
			case wp_beak: WeaponName = "Beak"; break;
			default: WeaponName = NULL; break;
		}
		
		if (!WeaponName)
			continue;
		
		fprintf(OutRMOD, "Weapon \"%s\"\r\n", WeaponName);
		fprintf(OutRMOD, "{\r\n");
		
		// Normal
		/*if (wpnlev1info[i].ammo != am_noammo || wpnlev1info[i].ammopershoot)
		{*/
			fprintf(OutRMOD, "\tInventory\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			// Ammo
			if (wpnlev1info[i].ammo != am_noammo)
				fprintf(OutRMOD, "\t\tAmmo \"%s\";\r\n", AmmoName);
			if (wpnlev1info[i].ammopershoot)
				fprintf(OutRMOD, "\t\tUsedAmmo %i;\r\n", wpnlev1info[i].ammopershoot);
			
			// Spawning
			if (i == wp_fist || i == wp_pistol)
				fprintf(OutRMOD, "\t\tSpawnWithInDoom true;\r\n");
			else if (i == wp_staff || i == wp_goldwand)
				fprintf(OutRMOD, "\t\tSpawnWithInHeretic true;\r\n");
			
			// Cheats
			if (i < wp_staff)
				fprintf(OutRMOD, "\t\tCanIDFA true;\r\n");
			else
				fprintf(OutRMOD, "\t\tCanRambo true;\r\n");
				
			// Primary Slot
			switch (i)
			{
				case wp_fist:
				case wp_chainsaw:
				case wp_staff:
				case wp_gauntlets:
					fprintf(OutRMOD, "\t\tPrimarySlot 1;\r\n");
					break;
				case wp_pistol:
				case wp_goldwand:
					fprintf(OutRMOD, "\t\tPrimarySlot 2;\r\n");
					break;
				case wp_shotgun:
				case wp_supershotgun:
				case wp_crossbow:
					fprintf(OutRMOD, "\t\tPrimarySlot 3;\r\n");
					break;
				case wp_chaingun:
				case wp_blaster:
					fprintf(OutRMOD, "\t\tPrimarySlot 4;\r\n");
					break;
				case wp_missile:
				case wp_skullrod:
					fprintf(OutRMOD, "\t\tPrimarySlot 5;\r\n");
					break;
				case wp_plasma:
				case wp_phoenixrod:
					fprintf(OutRMOD, "\t\tPrimarySlot 6;\r\n");
					break;
				case wp_bfg:
				case wp_mace:
					fprintf(OutRMOD, "\t\tPrimarySlot 7;\r\n");
					break;
				default:
					break;
			}
			
			// Secondary Slot
			switch (i)
			{
				case wp_chainsaw:
				case wp_gauntlets:
					fprintf(OutRMOD, "\t\tSecondarySlot 8;\r\n");
				default:
					break;
			}
				
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		/*}*/
		
		// Display
		if (i > 0 && i != 7 && i <= 8)
		{
			fprintf(OutRMOD, "\tDisplay\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			fprintf(OutRMOD, "\t\tOverlayIcon \"SBOAMMO%i\";\r\n", i);
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Super
		if ((wpnlev2info[i].upstate != wpnlev1info[i].upstate ||
			wpnlev2info[i].downstate != wpnlev1info[i].downstate ||
			wpnlev2info[i].readystate != wpnlev1info[i].readystate ||
			wpnlev2info[i].atkstate != wpnlev1info[i].atkstate ||
			wpnlev2info[i].holdatkstate != wpnlev1info[i].holdatkstate ||
			wpnlev2info[i].flashstate != wpnlev1info[i].flashstate) &&
		  (wpnlev2info[i].ammo != am_noammo || wpnlev2info[i].ammopershoot))
		{
			fprintf(OutRMOD, "\tSuperInventory\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			if (wpnlev2info[i].ammo != am_noammo)
				fprintf(OutRMOD, "\t\tSuperAmmo \"%s\";\r\n", AmmoName);
			if (wpnlev2info[i].ammopershoot)
				fprintf(OutRMOD, "\t\tSuperUsedAmmo %i;\r\n", wpnlev2info[i].ammopershoot);
				
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Normal states
		if (wpnlev1info[i].upstate || wpnlev1info[i].downstate || wpnlev1info[i].readystate || wpnlev1info[i].atkstate || wpnlev1info[i].holdatkstate || wpnlev1info[i].flashstate)
		{
			fprintf(OutRMOD, "\tStates\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			statecheck = NULL;
			
			tenstatecheck[0] = &wpnlev1info[i].upstate;
			tenstatecheck[1] = &wpnlev1info[i].downstate;
			tenstatecheck[2] = &wpnlev1info[i].readystate;
			tenstatecheck[3] = &wpnlev1info[i].atkstate;
			tenstatecheck[4] = &wpnlev1info[i].holdatkstate;
			tenstatecheck[5] = &wpnlev1info[i].flashstate;
			
			// this is for whitespace control
			l = 0;
			for (k = 0; k < 6; k++)
			{
				switch (k)
				{
					case 0: if (wpnlev1info[i].upstate) l |= (1 << k); break;
					case 1: if (wpnlev1info[i].downstate) l |= (1 << k); break;
					case 2: if (wpnlev1info[i].readystate) l |= (1 << k); break;
					case 3: if (wpnlev1info[i].atkstate) l |= (1 << k); break;
					case 4: if (wpnlev1info[i].holdatkstate) l |= (1 << k); break;
					case 5: if (wpnlev1info[i].flashstate) l |= (1 << k); break;
					default: continue;
				}
			}
			
			memset(stateref, 0, sizeof(stateref));
			memset(stateowner, 0, sizeof(stateowner));

			for (k = 0; k < 6; k++)
			{
				stateptr = statecheck;
				
				switch (k)
				{
					case 0: statecheck = &wpnlev1info[i].upstate; statecheckname = "UpState"; break;
					case 1: statecheck = &wpnlev1info[i].downstate; statecheckname = "DownState"; break;
					case 2: statecheck = &wpnlev1info[i].readystate; statecheckname = "ReadyState"; break;
					case 3: statecheck = &wpnlev1info[i].atkstate; statecheckname = "AttackState"; break;
					case 4: statecheck = &wpnlev1info[i].holdatkstate; statecheckname = "HeldAttackState"; break;
					case 5: statecheck = &wpnlev1info[i].flashstate; statecheckname = "FlashState"; break;
					default: continue;
				}
				
				l &= ~(1 << k);
			
				if (*statecheck)
				{
					fprintf(OutRMOD, "\t\t%s\r\n", statecheckname);
					fprintf(OutRMOD, "\t\t{\r\n");
				
					// Count
					statecount = 0;
					stateseek = *statecheck;
				
					//memset(stateref, 0, sizeof(stateref));
					stateref[0] = 0;

					do 
					{
						statecount++;
						if (stateref[stateseek] == 0)
							stateref[stateseek] = statecount;
						//else
						//	break;
						
						if (stateowner[stateseek] == 0)
							stateowner[stateseek] = k + 1;
						//else
						//	break;
						
						stateseek = states[stateseek].nextstate;
					}
					while (states[stateseek].tics != -1 && 
						stateseek != 0//states[stateseek].nextstate != 0
						&& !stateref[stateseek] && !stateowner[stateseek]);
					
					if (states[stateseek].tics == -1)
					{
						statecount++;
						stateref[stateseek] = statecount;
						stateowner[stateseek] = k + 1;
					}
					
					// Deploy
					stateseek = *statecheck;
				
					for (j = 0; j < statecount; j++)
					{
						if (stateseek == 0)
							continue;
						
						/* Normal statement */
						m = 0;
						if (stateowner[stateseek] == (k + 1))		// State MAY be a duplicate of another state
						{
							m = 1;
							
							fprintf(OutRMOD, "\t\t\tID %i, Sprite \"%s%s\":%i",
								j + 1,
								sprnames[states[stateseek].sprite],				// Sprite Name
								(states[stateseek].frame & 32768 ? "+" : ""),	// Symbols
								states[stateseek].frame & 32767				// Exclude upper bits
								);
						
							// Next or Next to a Goto?
							if (states[stateseek].tics != -1 && states[stateseek].nextstate != 0 &&
								stateowner[states[stateseek].nextstate] != k + 1)
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									statecount + 1,
									states[stateseek].tics
									);
							else
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									(states[stateseek].nextstate ? stateref[states[stateseek].nextstate] : 0),
									states[stateseek].tics
									);
					
							// Exec
							if (states[stateseek].action.acv)
								fprintf(OutRMOD, ", Exec \"%s\"", ReturnCodePointer(states[stateseek].action.acv));
					
							// DeHackEd
							if (stateseek >= S_FREETARGMOBJ)
							{
								actualdeh = stateseek;
								actualdeh -= S_FREETARGMOBJ;
					
								fprintf(OutRMOD, ", HereticDEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
							else
							{
								actualdeh = stateseek;
								fprintf(OutRMOD, ", DEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
					
							fprintf(OutRMOD, ";\r\n");
						}
					
						if (states[stateseek].tics == -1)
							break;
						
						stateseek = states[stateseek].nextstate;
					
						/* Check For Goto */
						if (stateowner[stateseek] != (k + 1))
						{
							switch (stateowner[stateseek] - 1)
							{
								case 0: doublecheckname = "UpState"; break;
								case 1: doublecheckname = "DownState"; break;
								case 2: doublecheckname = "ReadyState"; break;
								case 3: doublecheckname = "AttackState"; break;
								case 4: doublecheckname = "HeldAttackState"; break;
								case 5: doublecheckname = "FlashState"; break;
								case 10 + 0: doublecheckname = "SuperUpState"; break;
								case 10 + 1: doublecheckname = "SuperDownState"; break;
								case 10 + 2: doublecheckname = "SuperReadyState"; break;
								case 10 + 3: doublecheckname = "SuperAttackState"; break;
								case 10 + 4: doublecheckname = "SuperHeldAttackState"; break;
								case 10 + 5: doublecheckname = "SuperFlashState"; break;
								default: doublecheckname = NULL; break;
							}
					
							if (doublecheckname)
							{
								j++;
								
								if (stateref[stateseek] - (m ? 1 : 2))
									snprintf(nextnum, 5, "+%i", stateref[stateseek] - (m ? 1 : 2));
								else
									nextnum[0] = 0;
								
								fprintf(OutRMOD, "\t\t\tID %i, Goto \"%s\"%s;\r\n",
									j + (m ? 1 : 0),
								
									// Goto
									doublecheckname,
									nextnum
									);
								break;
							}
						}
						// ---
					}
				
					fprintf(OutRMOD, "\t\t}\r\n");
					
					if (l)
						fprintf(OutRMOD, "\t\t\r\n");
				}
			}
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		// Normal states
		if (wpnlev2info[i].upstate != wpnlev1info[i].upstate ||
			wpnlev2info[i].downstate != wpnlev1info[i].downstate ||
			wpnlev2info[i].readystate != wpnlev1info[i].readystate ||
			wpnlev2info[i].atkstate != wpnlev1info[i].atkstate ||
			wpnlev2info[i].holdatkstate != wpnlev1info[i].holdatkstate ||
			wpnlev2info[i].flashstate != wpnlev1info[i].flashstate)
		{
			fprintf(OutRMOD, "\tSuperStates\r\n");
			fprintf(OutRMOD, "\t{\r\n");
			
			statecheck = NULL;
			
			tenstatecheck[10 + 0] = &wpnlev2info[i].upstate;
			tenstatecheck[10 + 1] = &wpnlev2info[i].downstate;
			tenstatecheck[10 + 2] = &wpnlev2info[i].readystate;
			tenstatecheck[10 + 3] = &wpnlev2info[i].atkstate;
			tenstatecheck[10 + 4] = &wpnlev2info[i].holdatkstate;
			tenstatecheck[10 + 5] = &wpnlev2info[i].flashstate;
			
			// this is for whitespace control
			l = 0;
			for (k = 10 + 0; k < 10 + 6; k++)
			{
				switch (k)
				{
					case 10 + 0: if (wpnlev2info[i].upstate != wpnlev1info[i].upstate) l |= (1 << k); break;
					case 10 + 1: if (wpnlev2info[i].downstate != wpnlev1info[i].downstate) l |= (1 << k); break;
					case 10 + 2: if (wpnlev2info[i].readystate != wpnlev1info[i].readystate) l |= (1 << k); break;
					case 10 + 3: if (wpnlev2info[i].atkstate != wpnlev1info[i].atkstate) l |= (1 << k); break;
					case 10 + 4: if (wpnlev2info[i].holdatkstate != wpnlev1info[i].holdatkstate) l |= (1 << k); break;
					case 10 + 5: if (wpnlev2info[i].flashstate != wpnlev1info[i].flashstate) l |= (1 << k); break;
					default: continue;
				}
			}
			
			//memset(stateref, 0, sizeof(stateref));
			//memset(stateowner, 0, sizeof(stateowner));

			for (k = 10 + 0; k < 10 + 6; k++)
			{
				stateptr = statecheck;
				
				switch (k)
				{
					case 10 + 0:
						if (wpnlev2info[i].upstate != wpnlev1info[i].upstate)
							{statecheck = &wpnlev2info[i].upstate; statecheckname = "SuperUpState"; break;}
						else
							continue;
					case 10 + 1:
						if (wpnlev2info[i].downstate != wpnlev1info[i].downstate)
							{statecheck = &wpnlev2info[i].downstate; statecheckname = "SuperDownState"; break;}
						else
							continue;
					case 10 + 2:
						if (wpnlev2info[i].readystate != wpnlev1info[i].readystate)
							{statecheck = &wpnlev2info[i].readystate; statecheckname = "SuperReadyState"; break;}
						else
							continue;
					case 10 + 3:
						if (wpnlev2info[i].atkstate != wpnlev1info[i].atkstate)
							{statecheck = &wpnlev2info[i].atkstate; statecheckname = "SuperAttackState"; break;}
						else
							continue;
					case 10 + 4:
						if (wpnlev2info[i].holdatkstate != wpnlev1info[i].holdatkstate)
							{statecheck = &wpnlev2info[i].holdatkstate; statecheckname = "SuperHeldAttackState"; break;}
						else
							continue;
					case 10 + 5:
						if (wpnlev2info[i].flashstate != wpnlev1info[i].flashstate)
							{statecheck = &wpnlev2info[i].flashstate; statecheckname = "SuperFlashState"; break;}
						else
							continue;
					default: continue;
				}
				
				l &= ~(1 << k);
			
				if (*statecheck)
				{
					fprintf(OutRMOD, "\t\t%s\r\n", statecheckname);
					fprintf(OutRMOD, "\t\t{\r\n");
				
					// Count
					statecount = 0;
					stateseek = *statecheck;
				
					//memset(stateref, 0, sizeof(stateref));
					stateref[0] = 0;

					do 
					{
						statecount++;
						if (stateref[stateseek] == 0)
							stateref[stateseek] = statecount;
						//else
						//	break;
						
						if (stateowner[stateseek] == 0)
							stateowner[stateseek] = k + 1;
						//else
						//	break;
						
						stateseek = states[stateseek].nextstate;
					}
					while (states[stateseek].tics != -1 && 
						stateseek != 0//states[stateseek].nextstate != 0
						&& !stateref[stateseek] && !stateowner[stateseek]);
					
					if (states[stateseek].tics == -1)
					{
						statecount++;
						stateref[stateseek] = statecount;
						stateowner[stateseek] = k + 1;
					}
					
					// Deploy
					stateseek = *statecheck;
				
					for (j = 0; j < statecount; j++)
					{
						if (stateseek == 0)
							continue;
						
						/* Normal statement */
						m = 0;
						if (stateowner[stateseek] == (k + 1))		// State MAY be a duplicate of another state
						{
							m = 1;
							
							fprintf(OutRMOD, "\t\t\tID %i, Sprite \"%s%s\":%i",
								j + 1,
								sprnames[states[stateseek].sprite],				// Sprite Name
								(states[stateseek].frame & 32768 ? "+" : ""),	// Symbols
								states[stateseek].frame & 32767				// Exclude upper bits
								);
						
							// Next or Next to a Goto?
							if (states[stateseek].tics != -1 && states[stateseek].nextstate != 0 &&
								stateowner[states[stateseek].nextstate] != k + 1)
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									statecount + 1,
									states[stateseek].tics
									);
							else
								fprintf(OutRMOD, ", Next %i:%i",
									// Next
									(states[stateseek].nextstate ? stateref[states[stateseek].nextstate] : 0),
									states[stateseek].tics
									);
					
							// Exec
							if (states[stateseek].action.acv)
								fprintf(OutRMOD, ", Exec \"%s\"", ReturnCodePointer(states[stateseek].action.acv));
					
							// DeHackEd
							if (stateseek >= S_FREETARGMOBJ)
							{
								actualdeh = stateseek;
								actualdeh -= S_FREETARGMOBJ;
					
								fprintf(OutRMOD, ", HereticDEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
							else
							{
								actualdeh = stateseek;
								fprintf(OutRMOD, ", DEH \"%s\":%i",
									StateNames[stateseek],
									actualdeh);
							}
					
							fprintf(OutRMOD, ";\r\n");
						}
					
						if (states[stateseek].tics == -1)
							break;
						
						stateseek = states[stateseek].nextstate;
					
						/* Check For Goto */
						if (stateowner[stateseek] != (k + 1))
						{
							switch (stateowner[stateseek] - 1)
							{
								case 0: doublecheckname = "UpState"; break;
								case 1: doublecheckname = "DownState"; break;
								case 2: doublecheckname = "ReadyState"; break;
								case 3: doublecheckname = "AttackState"; break;
								case 4: doublecheckname = "HeldAttackState"; break;
								case 5: doublecheckname = "FlashState"; break;
								case 10 + 0: doublecheckname = "SuperUpState"; break;
								case 10 + 1: doublecheckname = "SuperDownState"; break;
								case 10 + 2: doublecheckname = "SuperReadyState"; break;
								case 10 + 3: doublecheckname = "SuperAttackState"; break;
								case 10 + 4: doublecheckname = "SuperHeldAttackState"; break;
								case 10 + 5: doublecheckname = "SuperFlashState"; break;
								default: doublecheckname = NULL; break;
							}
					
							if (doublecheckname)
							{
								j++;
								
								if (stateref[stateseek] - (m ? 1 : 2))
									snprintf(nextnum, 5, "+%i", stateref[stateseek] - (m ? 1 : 2));
								else
									nextnum[0] = 0;
								
								fprintf(OutRMOD, "\t\t\tID %i, Goto \"%s\"%s;\r\n",
									j + (m ? 1 : 0),
								
									// Goto
									doublecheckname,
									nextnum
									);
								break;
							}
						}
						// ---
					}
				
					fprintf(OutRMOD, "\t\t}\r\n");
					
					if (l)
						fprintf(OutRMOD, "\t\t\r\n");
				}
			}
			
			fprintf(OutRMOD, "\t}\r\n");
			fprintf(OutRMOD, "\t\r\n");
		}
		
		fprintf(OutRMOD, "\tLegacy\r\n");
		fprintf(OutRMOD, "\t{\r\n");
		
		fprintf(OutRMOD, "\t\tDeHackEdNum %i;\r\n", (i < wp_staff ? i : i - wp_staff));
		
		fprintf(OutRMOD, "\t}\r\n");
		
		fprintf(OutRMOD, "}\r\n");
		fprintf(OutRMOD, "\r\n");
	}
	
	fclose(OutRMOD);
	
	return 0;
}

