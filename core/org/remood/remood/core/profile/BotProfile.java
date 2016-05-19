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
 * This is a profile which is associated with a bot player. These profiles
 * mostly exist for the server and the local player playing the game so they
 * can save and use bots they already setup. Bots may have configurations
 * attached to them (such as aggression, defense, or their weapon order) and
 * this may store them.
 *
 * If a bot is not saved then the profile is lost at the end of the game,
 * however if it is saved then the bot can be restored with its previous
 * settings rather than setting up new bots.
 *
 * @since 2016/05/10
 */
public class BotProfile
	extends SavedProfile
{
}

