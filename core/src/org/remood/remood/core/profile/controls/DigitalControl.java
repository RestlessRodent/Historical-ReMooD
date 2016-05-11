// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.profile.controls;

/**
 * This is a digital control which is either on or off and is mapped to keys
 * and buttons.
 *
 * @since 2016/05/11
 */
public enum DigitalControl
	implements BaseControl
{
	/* Increase player speed (run). */
	SPEED,
	
	/** Strafe instead of turning. */
	MOVEMENT,
	
	/** Looking instead of moving. */
	LOOKING,
	
	/** Walk forwards. */
	FORWARDS,
	
	/** Walk backwards. */
	BACKWARDS,
	
	/** Strafe left. */
	STRAFE_LEFT,
	
	/** Strafe right. */
	STRAFE_RIGHT,
	
	/** Fly/Swim up. */
	FLY_SWIM_UP,
	
	/** Fly/Swim down. */
	FLY_SWIM_DOWN,
	
	/** Land (or swim down). */
	LAND,
	
	/** Jump (or swim up). */
	JUMP,
	
	/** Turn left */
	TURN_LEFT,
	
	/** Turn right. */
	TURN_RIGHT,
	
	/** Turn 180 degrees and go right. */
	TURN_180_CLOCKWISE,
	
	/** Turn 180 degrees and go left. */
	TURN_180_COUNTERCLOCKWISE,
	
	/** Look up. */
	LOOK_UP,
	
	/** Look entirely up. */
	LOOK_COMPLETELY_UP,
	
	/** Look down. */
	LOOK_DOWN,
	
	/** Look entirely down. */
	LOOK_COMPLETELY_DOWN,
	
	/** Center look .*/
	LOOK_CENTER,
	
	/** Use a door or switch. */
	USE,
	
	/** Is never the answer. */
	SUICIDE,
	
	/** Laugh in the face of your enemy. */
	TAUNT,
	
	/** Talk to others. */
	CHAT,
	
	/** Talk only to team members. */
	TEAM_CHAT,
	
	/** Attack. */
	ATTACK,
	
	/** Secondary function. */
	ALT_ATTACK,
	
	/** Reload the current weapon. */
	RELOAD,
	
	/** Make the secondary function the primary attack. */
	SWITCH_FIRE_MODE,
	
	/** Switch to weapon slot 1. */
	SLOT1,
	
	/** Switch to weapon slot 2. */
	SLOT2,
	
	/** Switch to weapon slot 3. */
	SLOT3,
	
	/** Switch to weapon slot 4. */
	SLOT4,
	
	/** Switch to weapon slot 5. */
	SLOT5,
	
	/** Switch to weapon slot 6. */
	SLOT6,
	
	/** Switch to weapon slot 7. */
	SLOT7,
	
	/** Switch to weapon slot 8. */
	SLOT8,
	
	/** Switch to weapon slot 9. */
	SLOT9,
	
	/** Switch to weapon slot 10. */
	SLOT10,
	
	/** Go to the next weapon. */
	NEXT_WEAPON,
	
	/** Go to the previous weapon. */
	PREV_WEAPON,
	
	/** Switch to the best weapon. */
	BEST_WEAPON,
	
	/** Switch to the worst weapon. */
	WORST_WEAPON,
	
	/** Select next inventory item. */
	NEXT_INVENTORY,
	
	/** Select previous inventory item. */
	PREV_INVENTORY,
	
	/** Use the current inventory item. */
	USE_INVENTORY,
	
	/** Stop looking at the inventory. */
	CANCEL_INVENTORY,
	
	/** Show the best players.  */
	TOP_SCORES,
	
	/** Show the worst players. */
	BOTTOM_SCORES,
	
	/** Show allied players. */
	ALLIES,
	
	/** Spy the next player. */
	COOPSPY_NEXT,
	
	/** Spy the previous player. */
	COOPSPY_PREV,
	
	/** Stop spying players and view through your eyes. */
	COOPSPY_STOP,
	
	/** Toggle the automap. */
	AUTOMAP,
	
	/** Show the player menu. */
	POPUP_MENU,
	
	/** Quick item selection menu (similar to Perfect Dark). */
	QUICK_MENU,
	
	/** Binding mask. */
	MORE_STUFF,
	
	/** Additional binding mask. */
	MORE_MORE_STUFF,
	
	/** End. */
	;
}

