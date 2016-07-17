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
 * This identifies a device driver.
 *
 * @since 2016/07/17
 */
public final class DeviceDriverIdentity
{
	/** The name of the driver. */
	protected final String name;
	
	/** The priority of the driver. */
	protected final int priority;
	
	/**
	 * Initializes the identification.
	 *
	 * @param __n The driver name.
	 * @param __p The driver priority.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/17
	 */
	public DeviceDriverIdentity(String __n, int __p)
		throws NullPointerException
	{
		// Check
		if (__n == null)
			throw new NullPointerException();
		
		// Set
		this.name = __n;
		this.priority = __p;
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	public final boolean equals(Object __o)
	{
		// Check
		if (!(__o instanceof DeviceDriverIdentity))
			return false;
		
		// Cast
		DeviceDriverIdentity o = (DeviceDriverIdentity)__o;
		return this.name.equals(o.name) &&
			this.priority == o.priority;
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	public final int hashCode()
	{
		return this.name.hashCode() ^ this.priority;
	}
	
	/**
	 * Returns the name of the device driver.
	 *
	 * @return The driver name.
	 * @since 2016/07/17
	 */
	public final String name()
	{
		return this.name;
	}
	
	/**
	 * Returns the priority of the device driver.
	 *
	 * @return The driver priority.
	 * @since 2016/07/17
	 */
	public final int priority()
	{
		return this.priority;
	}
}

