// -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------
// DESCRIPTION: Converts a pic_t to a P6 PPM

/***************
*** INCLUDES ***
***************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************
*** STRUCTURES ***
*****************/

/* pic_t -- Legacy Picture */
typedef struct pic_s
{
	int16_t width;
	uint8_t zero;					// set to 0 allow autodetection of pic_t 
	// mode instead of patch or raw
	uint8_t mode;					// see pic_mode_t above
	int16_t height;
	int16_t reserved1;			// set to 0
} pic_t;

/*************
*** LOCALS ***
*************/

uint8_t PlayPal[256][3];

/****************
*** FUNCTIONS ***
****************/

/* main() -- Program entry point */
int main(int argc, char** argv)
{
#define BUFSIZE 512
	char OutName[BUFSIZE];
	FILE* inPIC;
	FILE* outPPM;
	FILE* pp;
	pic_t Header;
	uint8_t u8;
	size_t i, j;
	
	/* Load playpal.lmp */
	pp = fopen("playpal.lmp", "rb");
	if (pp)
	{
		fseek(pp, 0, SEEK_SET);
		for (i = 0; i < 256; i++)
			for (j = 0; j < 3; j++)
				fread(&PlayPal[i][j], 1, 1, pp);
		fclose(pp);
	}
	
	/* Check arguments */
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <pic_t.lmp>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	/* Open input pic_t */
	inPIC = fopen(argv[1], "rb");
	
	if (!inPIC)
		return EXIT_FAILURE;
	
	/* Open output ppm */
	strncpy(OutName, argv[1], BUFSIZE);
	strncat(OutName, ".ppm", BUFSIZE);
	outPPM = fopen(OutName, "wb");
	
	/* Seek to start */
	fseek(inPIC, 0, SEEK_SET);
	
	// Read in header
	fread(&Header.width, 2, 1, inPIC);
	fread(&Header.zero, 1, 1, inPIC);
	fread(&Header.mode, 1, 1, inPIC);
	fread(&Header.height, 2, 1, inPIC);
	fread(&Header.reserved1, 2, 1, inPIC);
	
	// Write output PPM
	fprintf(outPPM, "P6\n%i %i\n255\n", Header.width, Header.height);
	
	// Start copying the data
	for (i = 0; i < Header.width * Header.height; i++)
	{
		// Read in picture
		fread(&u8, 1, 1, inPIC);
		
		// Write out RGB
		fwrite(&PlayPal[u8][0], 1, 1, outPPM);
		fwrite(&PlayPal[u8][1], 1, 1, outPPM);
		fwrite(&PlayPal[u8][2], 1, 1, outPPM);
	}
	
	/* Close both */
	fclose(inPIC);
	fclose(outPPM);
	
	/* Success */
	return EXIT_SUCCESS;
#undef BUFSIZE
}

