/* Convert ASCII/Hex bitstream data to actual bytes... each pair of hex characters is translated to binary */

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int main(void)
{
	unsigned int u;
	while(scanf("%2x", &u) == 1)
	{
		/* Please... a char be a byte! */
		unsigned char b = (unsigned char)u;
		fwrite(&b, 1, 1, stdout);
	}
	return 0;
}
