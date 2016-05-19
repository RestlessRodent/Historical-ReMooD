// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system;

/**
 * This class implements a dynamically event queue which is used for handling
 * system based events such as the keyboard, mouse, and joystick.
 *
 * Events are attached to controller ports (which means say on a hypothetical
 * Dreamcast port, there could be 4 keyboards attached with 4 player.
 *
 * @since 2016/05/19
 */
public class InputEvents
{
	/**
	 * This is the type of event which was passed.
	 *
	 * @since 2016/05/19
	 */
	public enum Type
	{
		/** End. */
		;
	}
}

