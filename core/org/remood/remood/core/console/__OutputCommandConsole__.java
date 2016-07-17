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
 * This is an output console which writes any input lines to any passed
 * output streams.
 *
 * @since 2016/07/16
 */
class __OutputCommandConsole__
	extends OutputStream
{
	/** The owning game console. */
	protected final GameConsole console;
	
	/** Output consoles to forward to. */
	private final OutputStream[] _output;
	
	/**
	 * Initializes the output command console.
	 *
	 * @param __gc The owning console.
	 * @param __output Output consoles which are given text, may be used
	 * for logging.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/16
	 */
	__OutputCommandConsole__(GameConsole __gc, OutputStream... __output)
		throws NullPointerException
	{
		// Check
		if (__gc == null)
			throw new NullPointerException();
		
		// Set
		this.console = __gc;
		this._output = (__output == null ? new OutputStream[0] :
			__output.clone());
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/16
	 */
	@Override
	public void write(int __b)
	{
		throw new Error("TODO");
	}
}

