/* Convert ASCII/Hex bitstream data to actuall bytes... each pair of hex characters is translated to binary */

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int main()
{
	unsigned int u;
	while(scanf("%2x", &u) == 1)
	{
		/* Please... a char be a byte! */
		unsigned char b = (unsigned char)u;
		write(STDOUT_FILENO, &b, 1);
	}
	return 0;
}
