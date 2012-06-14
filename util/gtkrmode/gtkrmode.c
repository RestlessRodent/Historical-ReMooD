// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// -----------------------------------------------------------------------------
//         :oCCCCOCoc.
//     .cCO8OOOOOOOOO8Oo:
//   .oOO8OOOOOOOOOOOOOOOCc
//  cO8888:         .:oOOOOC.                                                TM
// :888888:   :CCCc   .oOOOOC.     ###      ###                    #########
// C888888:   .ooo:   .C########   #####  #####  ######    ######  ##########
// O888888:         .oO###    ###  #####  ##### ########  ######## ####    ###
// C888888:   :8O.   .C##########  ### #### ### ##    ##  ##    ## ####    ###
// :8@@@@8:   :888c   o###         ### #### ### ########  ######## ##########
//  :8@@@@C   C@@@@   oo########   ###  ##  ###  ######    ######  #########
//    cO@@@@@@@@@@@@@@@@@Oc0
//      :oO8@@@@@@@@@@Oo.
//         .oCOOOOOCc.                                      http://remood.org/
// -----------------------------------------------------------------------------
// Copyright (C) 2011-2012 GhostlyDeath <ghostlydeath@remood.org>
// -----------------------------------------------------------------------------
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -----------------------------------------------------------------------------
// DESCRIPTION: GTK REMOODAT Editor

/***************
*** INCLUDES ***
***************/

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

/*****************
*** STRUCTURES ***
*****************/

/* MainWindowJunk_t -- Stuffed used by the main window */
typedef struct MainWindowJunk_s
{
	GtkWidget* Self;							// Self window widget
	GtkWidget* VBox;							// Split box
	GtkWidget* MenuBar;							// Menu Bar
	
	GtkWidget* IOptREMOODAT;					// REMOODAT Item
	GtkWidget* MOptREMOODAT;					// REMOODAT Menu
} MainWindowJunk_t;

/****************
*** FUNCTIONS ***
****************/

/* CreateMainWindow() -- Creates Main GTK Window */
GtkWidget* CreateMainWindow(void)
{
	MainWindowJunk_t* New;
	
	/* Allocate Junk */
	New = malloc(sizeof(*New));
	memset(New, 0, sizeof(*New));
	
	/* Setup Window */
	New->Self = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(New->Self), "GTK REMOODAT Editor");
	
	/* Create Primary Split */
	New->VBox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(New->Self), New->VBox);
	gtk_widget_show(New->VBox);
	
	/* Create MenuBar */
	New->MenuBar = gtk_menu_bar_new();
	gtk_container_add(GTK_CONTAINER(New->VBox), New->MenuBar);
	gtk_widget_show(New->MenuBar);
	
	// Add REMOODAT Menu
	New->MOptREMOODAT = gtk_menu_new();
	New->IOptREMOODAT = gtk_menu_item_new_with_mnemonic("_REMOODAT");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(New->IOptREMOODAT), New->MOptREMOODAT);
	gtk_menu_shell_append(GTK_MENU_SHELL(New->MenuBar), New->IOptREMOODAT);
	gtk_widget_show(New->IOptREMOODAT);
	
	/* Return Window */
	return New->Self;
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	GtkWidget* MainWindow;
	
	/* Initialize GTK */
	gtk_init(&argc, &argv);
	
	MainWindow = CreateMainWindow();
	gtk_widget_show(MainWindow);
	
	/* GTK Loop */
	gtk_main();
}



