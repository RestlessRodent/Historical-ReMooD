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
		BitData[j++] = b;
	}
	ll = j;
	
	// Print packed data
	for (k = 0; k < ll; k++)
	{
		fprintf(fo, "%hhu,", BitData[k]);
		if (((k + 1) % 30) == 0)
			fprintf(fo, "\n");
	}
	fprintf(stderr, "SIZE: %i\n", (int)ll);
}
