// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Poorly Written NCurses ENDOOM Editor

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static uint8_t l_Bits[80 * 25];
static uint8_t l_Attr[80 * 25];
static uint8_t l_Mode;

static const uint8_t c_ColorToCurse[8] =
{
	COLOR_BLACK, // Black
	COLOR_BLUE, // Blue
	COLOR_GREEN, // Green
	COLOR_CYAN, // Cyan
	COLOR_RED, // Red
	COLOR_MAGENTA, // Meganta
	COLOR_YELLOW, // Brown
	COLOR_WHITE, // White
};

static const struct
{
	const char Key;
	const int ModType;	// 0 == fg, 1 == bg, 2 == blink
	const int Value;		// new value
} c_ColorChoice[] =
{
	// FG
	{'q', 0, 0},
	{'w', 0, 1},
	{'e', 0, 2},
	{'r', 0, 3},
	{'t', 0, 4},
	{'y', 0, 5},
	{'u', 0, 6},
	{'i', 0, 7},
	{'a', 0, 8},
	{'s', 0, 9},
	{'d', 0, 10},
	{'f', 0, 11},
	{'g', 0, 12},
	{'h', 0, 13},
	{'j', 0, 14},
	{'k', 0, 15},
	
	// BG
	{'1', 1, 0},
	{'2', 1, 1},
	{'3', 1, 2},
	{'4', 1, 3},
	{'5', 1, 4},
	{'6', 1, 5},
	{'7', 1, 6},
	{'8', 1, 7},
	
	// Blink
	{'b', 2, 1},
	{'n', 2, 0},
	
	// End
	{0, 0, 0},
};

static int FullReverse = 0;

/* DrawInst() -- Draw instructions */
void DrawInst(void)
{
	size_t i;
	int x;
	
	/* Draw loop */
	for (i = 0; c_ColorChoice[i].Key; i++)
	{
		// Reset attributes
		standend();
		
		// Which type of instruction is this?
		if (c_ColorChoice[i].ModType == 0)
			mvaddch(27, i, 'F');	// FG
		else if (c_ColorChoice[i].ModType == 1)
			mvaddch(27, i, 'B');	// BG
		else if (c_ColorChoice[i].ModType == 2)
			mvaddch(27, i, 'b');	// Blink
		else
			continue;
		
		// Drawing foreground color
		if (c_ColorChoice[i].ModType == 0)
		{
			x = (c_ColorChoice[i].Value & 0xF);
			
			if (x & 0x8)
				attron(A_BOLD);
			
			// Set color
			attron(COLOR_PAIR(x + 1));
		}
		if (c_ColorChoice[i].ModType == 1)
		{
			x = ((c_ColorChoice[i].Value & 0x7) << 4) | 7;
			
			// Set color
			attron(COLOR_PAIR(x + 1));
		}
		if (c_ColorChoice[i].ModType == 2)
		{
			// Make blink and underline
			attron(A_BLINK);
			attron(A_UNDERLINE);
		}
		
		// Draw character
		mvaddch(26, i, c_ColorChoice[i].Key);
	}
	
	mvprintw(26, i + 1, "\\ quit+save; / toggle full reverse; arrow keys move");
}

/* IsRecolor() -- Is this a recolor? */
int IsRecolor(int ch)
{
	int i;
	
	/* Look in loop */
	for (i = 0; c_ColorChoice[i].Key; i++)
		if (c_ColorChoice[i].Key == ch)
			return i;
	
	/* Nothing */
	return -1;
}

/* DrawChar() --  Draws character in buffer at it's location */
void DrawChar(int c, int r, int reset, int doreverse)
{
	uint8_t x;
	uint8_t fg, bg;
	
	/* Check */
	if (c < 0 || c >= 80 || r < 0 || r >= 25)
		return;
	
	/* revert */
	if (reset)
		standend();
	
	/* Full reverse? */
	if (doreverse)
	{
		attroff(A_REVERSE);
		if (FullReverse)
			attron(A_REVERSE);
		else
			attroff(A_REVERSE);
	}
	
	attroff(A_BLINK);
	attroff(A_UNDERLINE);
	attroff(A_BOLD);
	
	/* Do attribute first */
	x = l_Attr[(r * 80) + c];
	
	/* Enable blinking? */
	if (x & 0x80)
	{
		attron(A_BLINK);
		attron(A_UNDERLINE);
	}
	else
	{
		attroff(A_BLINK);
		attroff(A_UNDERLINE);
	}
	
	/* Enable bold? */
	if (x & 0x8)
		attron(A_BOLD);
	else
		attroff(A_BOLD);
	
	/* Which color? */
	fg = x & 0x7F;
	attron(COLOR_PAIR(fg + 1));
	
	/* Now draw character */
	x = l_Bits[(r * 80) + c];
	mvaddch(r, c, x);
}

/* CurseDraw() -- Draw in ncurses */
void CurseDraw(void)
{
	size_t c, r;
	uint8_t x;
	
	/* Draw everything */
	for (r = 0; r < 25; r++)
		for (c = 0; c < 80; c++)
			DrawChar(c, r, 1, 1);
}

/* main() -- Main entry point */
int main(int argc, char** argv)
{
	size_t c, r, i;
	FILE* f;
	FILE* o;
	char x;
	uint8_t z;
	int ch, rc;
	MEVENT mEvent;
	int mX = 0, mY = 0;
	int DrawColor;
	
	/* Check */
	if (argc < 2)
		return EXIT_FAILURE;
	
	/* Open File */
	f = fopen(argv[1], "rt");
	
	// Check
	if (!f)
		return EXIT_FAILURE;
	
	/* Fill with space */
	for (c = 0; c < 80; c++)
		for (r = 0; r < 25; r++)
			l_Bits[(r * 80) + c] = ' ';
			
	for (c = 0; c < 80; c++)
		for (r = 0; r < 25; r++)
			l_Attr[(r * 80) + c] = 0xF;
	
	/* Read file */
	// Template
	if (argc > 2)
	{
		o = fopen(argv[2], "rt");
		
		if (o)
		{
			c = r = 0;
			while (!feof(o))
			{
				// Get Char
				x = fgetc(f);
		
				// Newline?
				if (x == '\n')
				{
					c = 0;
					r++;
					continue;
				}
		
				// Invalid?
				if (x < ' ')
					continue;
		
				// Place in array
				l_Bits[(r * 80) + c] = x;
		
				// Increment c
				c++;
			}
			
			fclose(o);
		}
	}
	
	// Source
	else
	{
		c = 0;
		
		while (!feof(f))
		{
			// Read char
			fread(&z, 1, 1, f);
			
			// Put in stuff
			l_Bits[c] = z;
			
			// Read attr
			fread(&z, 1, 1, f);
			
			// Put in stuff
			l_Attr[c++] = z;
			
			// End?
			if (c == 4000)
				break;
		}
	}
		
	/* ncurses edit */
	// init
	initscr();
	start_color();
	noecho();
	cbreak();
	keypad(stdscr, true);
	mousemask(BUTTON1_CLICKED | REPORT_MOUSE_POSITION, NULL);
	
	// Create colors
	for (i = 0; i < 0x7F; i++)
		init_pair(i + 1, c_ColorToCurse[i & 0x7], c_ColorToCurse[(i >> 4) & 0x7]);
	
	for (;;)
	{
		// Enable nodelay for event getting
		nodelay(stdscr, true);
		
		// Get mouse position
		for (;;)
		{
			ch = getch();
			
			// No more events?
			if (ch == ERR)
				break;
				
			// Quit?
			if (ch == '\\')
				break;
			
			if (ch == '/')
				FullReverse = !FullReverse;
			
			// Change Mode
			if (ch == KEY_F(2))
			{
				l_Mode = !l_Mode;
				continue;
			}
			
			// Move cursor?
			if (ch == KEY_DOWN)
				mY++;
			if (ch == KEY_UP)
				mY--;
			if (ch == KEY_LEFT)
				mX--;
			if (ch == KEY_RIGHT)
				mX++;
				
			// Cap mouse
			if (mX < 0)
				mX = 0;
			if (mX >= 80)
				mX = 79;
			if (mY < 0)
				mY = 0;
			if (mY >= 25)
				mY = 24;
			
			// Attribute Mode
			if (!l_Mode)
			{
				// Is this a recolor?
				if ((rc = IsRecolor(ch)) >= 0)
				{
					// Change FG?
					if (c_ColorChoice[rc].ModType == 0)
						l_Attr[(mY * 80) + mX] = (l_Attr[(mY * 80) + mX] & (~0xF)) | c_ColorChoice[rc].Value;
				
					// Change BG?
					else if (c_ColorChoice[rc].ModType == 1)
					{
						l_Attr[(mY * 80) + mX] = (l_Attr[(mY * 80) + mX] & (~(0x7 << 4))) | (c_ColorChoice[rc].Value << 4);
					}
				
					// Change Blink?
					else if (c_ColorChoice[rc].ModType == 2)
					{
						if (c_ColorChoice[rc].Value)
							l_Attr[(mY * 80) + mX] |= 0x80;
						else
							l_Attr[(mY * 80) + mX] &= ~0x80;
					}
				}
			}
			
			// Character Mode
			else
			{
				if (ch >= ' ' && ch <= 0x7F)
					l_Bits[(mY * 80) + mX] = ch & 0x7F;
			}
		}
		
		// Draw
		CurseDraw();
		
		// Mouse pos
		if (FullReverse)
			attroff(A_REVERSE);
		else
			attron(A_REVERSE);
		DrawChar(mX, mY, 0, 0);
		attroff(A_REVERSE);
		standend();	// revert all specials
		mvprintw(25, 0, "Pos: %i, %i     %s      ", mX, mY, (l_Mode ? "char" : "attrib"));
		
		// Draw instructions
		DrawInst();
		
		// Refresh
		refresh();
		
		// Quit?
		if (ch == '\\')
			break;
		
		// Disable nodelay
		nodelay(stdscr, false);
		ch = getch();
		ungetch(ch);	// Undo
	}
	
	// End
	endwin();
	
	/* Close */
	fclose(f);
	
	/* Print out */
	o = fopen(argv[1], "wb");
	
	if (o)
	{
		for (r = 0; r < 25; r++)
			for (c = 0; c < 80; c++)
			{
				// Print character
				x = l_Bits[(r * 80) + c];
				fwrite(&x, 1, 1, o);
				
				// Print white on black
				x = l_Attr[(r * 80) + c];
				fwrite(&x, 1, 1, o);
				
			}
		
		fclose(o);
	}

	return EXIT_SUCCESS;
}

