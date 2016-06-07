// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system;

import java.util.ServiceLoader;

/**
 * This class is used for managing instances of drivers of a specific type.
 *
 * @since 2016/06/07
 */
public class DriverManager<D extends DeviceDriver>
{
	/** The loader for driver services. */
	protected final ServiceLoader<D> loader;
	
	/** The type of services to find. */
	protected final Class<D> type;
	
	/**
	 * Initializes the driver manager.
	 *
	 * @param __cl The class to lookup drivers for.
	 * @throws ClassCastException If the given class is not a driver.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/06/07
	 */
	public DriverManager(Class<D> __cl)
		throws ClassCastException, NullPointerException
	{
		// Check
		if (__cl == null)
			throw new NullPointerException();
		if (__cl.isAssignableFrom(DeviceDriver.class))
			throw new ClassCastException();
		
		// Setup loader
		this.type = __cl;
		this.loader = ServiceLoader.<D>load(__cl);
	}
}

