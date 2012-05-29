#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv)
{
	int rc;
	uint8_t b, x;
	size_t cc;
	FILE* fi = fopen(argv[1], "rb");
	FILE* fo = fopen("xbinx", "wb");
	
	cc = 0; x = 0;
	while ((rc = fread(&b, 1, 1, fi) > 0))
	{
		if (b == '1')
			x |= (1 << (cc++));
		else
			cc++;
		
		if (cc == 8)
		{
			fwrite(&x, 1, 1, fo);
			x = 0;
			cc = 0;
		}
	}
}
