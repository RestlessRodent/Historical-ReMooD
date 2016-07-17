// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.console;

import java.io.IOException;
import java.io.OutputStream;

/**
 * This is the input command console which parses commands and then executes
 * them either in a global system or for a specific game.
 *
 * @since 2016/07/16
 */
class __InputCommandConsole__
	extends OutputStream
{
	/** The owning game console. */
	protected final GameConsole console;
	
	/**
	 * Initializes the input command console.
	 *
	 * @param __gc The owning console.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/16
	 */
	__InputCommandConsole__(GameConsole __gc)
		throws NullPointerException
	{
		// Check
		if (__gc == null)
			throw new NullPointerException();
		
		// Set
		this.console = __gc;
	}
}

