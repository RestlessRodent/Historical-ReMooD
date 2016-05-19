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
 * This provides an interface for a single profile. Profiles are needed for
 * split-screen players because there is a very high possibility that they
 * require their own unique settings.
 *
 * Profiles are either local, remote, virtual (a remote player), or owned by a
 * bot.
 *
 * Local and bot profiles are saved to the local systems, remote profiles
 * require an internet connection to be up to date and may be shared across
 * multiple systems (they are hosted on remood.org).
 *
 * @since 2016/05/10
 */
public abstract class Profile
{
}

