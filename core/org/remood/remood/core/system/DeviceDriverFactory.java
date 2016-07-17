// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system;

import org.remood.remood.core.config.GameConfiguration;
import org.remood.remood.core.console.GameConsole;

/**
 * This is used to create instances of drivers as needed.
 *
 * @since 2016/07/17
 */
public interface DeviceDriverFactory<D extends DeviceDriver>
{
	/**
	 * Creates an instance of a new driver.
	 *
	 * @param __gc The console.
	 * @param __conf The game configuration.
	 * @return The given driver.
	 * @since 2016/07/17
	 */
	public abstract D createDriver(GameConsole __gc, GameConfiguration __conf);
	
	/**
	 * Returns the name of the device driver.
	 *
	 * @return The device driver name.
	 * @since 2016/05/19
	 */
	public abstract String name();
	
	/**
	 * How important is this driver?
	 *
	 * @return The priority of this driver, the higher the more likely it is
	 * to be chosen.
	 * @since 2016/07/16
	 */
	public abstract int priority();
	
	/**
	 * The type of class this implements support for.
	 *
	 * @since 2016/07/16
	 */
	public abstract Class<D> type();
}

