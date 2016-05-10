// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.profile;

/**
 * This is a profile which may be cached remotely and has time based change
 * indexes place on local settings. Remote profiles are stored on a central
 * master server (remood.org) and are obtained/created when logging in and
 * such. This means that if one plays on another computer system, they only
 * need to login to obtain all of their settings.
 *
 * @since 2016/05/10
 */
public class RemoteProfile
	extends ControlledProfile
{
}

