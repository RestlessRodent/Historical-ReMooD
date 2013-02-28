#include <stdio.h>
#include <math.h>

int main(int argc, char** argv)
{
	int i, v;
	double Unit, Cur;
	int up, low;
	
	/* Calc unit size */
#if 1
	Unit = 8192.0 / 127.0;
	
	for (i = -128; i < 128; i++)
	{
		// Current unit
		Cur = Unit * (double)i;
		v = roundf(Cur);
		
		// Add 8192 to normalize
		v += 8192;
		
		if (v > 16383)
			v = 16383;
		else if (v < 0)
			v = 0;
		
		up = (v >> 7) & 0x7F;
		low = v & 0x7F;
		
		printf("{0x%02x, 0x%02x}, ", up, low, v);
		
		if (((i+1) % 4) == 0)
			printf("\n");
	}
	
	printf("\n");
	
#else
	Unit = 16384.0 / 256.0;
	
	for (i = 0; i < 256; i++)
	{
		// Current unit
		Cur = Unit * (double)i;
		v = roundf(Cur);
		
		if (v > 16383)
			v = 16383;
		
		up = (v >> 7) & 0x7F;
		low = v & 0x7F;
		
		printf("{0x%02x, 0x%02x}, ", up, low, v);
		
		if (((i+1) % 4) == 0)
			printf("\n");
	}
	printf("\n");
#endif
	
	return 0;
}

