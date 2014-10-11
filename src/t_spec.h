// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: FraggleScript files...

#ifndef __SPEC_H__
#define __SPEC_H__

void spec_brace();

int spec_if();					//SoM: returns weather or not the if statement was true.
int spec_elseif(bool_t lastif);
void spec_else(bool_t lastif);
void spec_while();
void spec_for();
void spec_goto();

// variable types

bool_t spec_variable();

void spec_script();				// in t_script.c btw

#endif
