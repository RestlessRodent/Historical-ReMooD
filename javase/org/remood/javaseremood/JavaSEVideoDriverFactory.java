// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.javaseremood;

import org.remood.remood.core.system.DeviceDriverFactory;
import org.remood.remood.core.system.DeviceDriverIdentity;
import org.remood.remood.core.system.video.VideoDriverFactory;

/**
 * This driver implements the factory for the video drivers.
 *
 * @since 2016/07/17
 */
public class JavaSEVideoDriverFactory
	implements VideoDriverFactory
{
	/** The identity for this driver. */
	private static final DeviceDriverIdentity _IDENTITY =
		new DeviceDriverIdentity("javase-video", Integer.MAX_VALUE);
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	public final DeviceDriverIdentity identity()
	{
		return _IDENTITY;
	}
}

