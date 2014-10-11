// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: handle mouse/keyboard/joystick inputs,

#ifndef __G_INPUT_H__
#define __G_INPUT_H__






#define MAXMOUSESENSITIVITY   160	// sensitivity steps

// number of total 'button' inputs, include keyboard keys, plus virtual
// keys (mousebuttons and joybuttons becomes keys)
#define NUMKEYS         256

#define MOUSEBUTTONS    8

// In i_util.h and g_input.h
#ifndef JOYBUTTONS
#define JOYBUTTONS      32
#endif

//
// mouse and joystick buttons are handled as 'virtual' keys
//
#if (MAXSPLITS > 4)
#error MODIFY INPUTS (Max Splitscreen players is now > 4)!
#endif

#define MOUSEBUTTONRANGE ((MOUSEBUTTONS << 1) + 2)
#define JOYBUTTONRANGE (JOYBUTTONS << 1)

#endif							/* __G_INPUT_H__ */

