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
 * This is the base class for all device driver factories.
 *
 * @since 2016/07/17
 */
public interface DeviceDriverFactory
{
	/**
	 * Returns the identity of this device driver.
	 *
	 * @return The driver identity.
	 * @since 2016/07/17
	 */
	public abstract DeviceDriverIdentity identity();
}

