#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static uint8_t BitData[64000];
static uint8_t CompData[64000];

int main(int argc, char** argv)
{
	size_t i, j, zCount, k, ooo, z, ll;
	size_t sz;
	uint8_t b, OldB, CarryB;
	FILE* fi = fopen(argv[1], "rb");
	FILE* fo = fopen("cdump", "wt");
	
	fseek(fi, 0, SEEK_END);
	sz = ftell(fi);
	fseek(fi, 0, SEEK_SET);
	
	zCount = 0;
	OldB = 0;
	CarryB = 0;
	for (j = 0, i = 0; i < sz; i++)
	{
		fread(&b, 1, 1, fi);
		
		if ((k & 1) == 0)
			BitData[j] = b;
		else
			BitData[j++] |= b << 4;
		k++;
	}
	
	// Compress Data, somewhat (RLE style)
	zCount = 0;
	OldB = BitData[0];
	for (ll = 0, i = 1; i < j; i++)
	{
		b = BitData[i];
		
		// Does not match?
		if (OldB != b)
		{
			CompData[ll++] = zCount;
			CompData[ll++] = b;
			OldB = b;
			zCount = 0;
		}
		
		// Matches
		else
		{
			zCount++;
			
			if (zCount > 250)
				fprintf(stderr, "TOO MUCH!\n");
		}
	}
	
	// Print packed data
	for (k = 0; k < ll; k++)
	{
		fprintf(fo, "%hhu,", CompData[k]);
		if (((k + 1) % 30) == 0)
			fprintf(fo, "\n");
	}
	fprintf(stderr, "SIZE: %i\n", (int)ll);
}
