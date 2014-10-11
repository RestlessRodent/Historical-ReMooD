// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
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



