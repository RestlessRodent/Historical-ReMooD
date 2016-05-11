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
 * This represents an analog based control which is mapped to for example
 * mouse axis and joystick axis.
 *
 * @since 2016/05/11
 */
public enum AnalogControl
	implements BaseControl
{
	/** Move on the X axis. */
	MOVE_LEFTRIGHT,
	
	/** Move on the Y axis. */
	MOVE_FORWARDSBACKWARDS,
	
	/** Look on the X axis. */
	LOOK_LEFTRIGHT,
	
	/** Look on the Y axis. */
	LOOK_UPDOWN,
	
	/** Inverted X axis movement. */
	INVERTED_MOVE_LEFTRIGHT,
	
	/** Inverted Forward/backwards movement. */
	INVERTED_MOVE_FORWARDSBACKWARDS,
	
	/** Inverted look left/right. */
	INVERTED_LOOK_LEFTRIGHT,
	
	/** Inverted look up/down. */
	INVERTED_LOOK_UPDOWN,
	
	/** Joystick only: Linear pan up/down. */
	LINEAR_PAN_LOOK_UPDOWN,
	
	/** Joystick only: Inverted linear pan up/down. */
	INVERTED_LINEAR_PAN_LOOK_UPDOWN,
	
	/** Joystick only: Angular pan up/down. */
	ANGULAR_PAN_LOOK_UPDOWN,
	
	/** Joystick only: Inverted angular pan up/down. */
	INVERTED_ANGULAR_PAN_LOOK_UPDOWN,
	
	/** End. */
	;
}

