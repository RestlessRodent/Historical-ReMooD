// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system.video;

import org.remood.remood.core.config.GameConfiguration;
import org.remood.remood.core.console.GameConsole;
import org.remood.remood.core.system.DeviceDriverFactory;

/**
 * This is a factory which creates video drivers.
 *
 * @since 2016/07/17
 */
public interface VideoDriverFactory
	extends DeviceDriverFactory
{
	/**
	 * Creates a new video driver instance.
	 *
	 * @param __gc The game console.
	 * @param __conf The game configuration.
	 * @return The instance of the driver.
	 * @since 2016/07/17
	 */
	public abstract VideoDriver createDriver(GameConsole __gc,
		GameConfiguration __conf);
}

