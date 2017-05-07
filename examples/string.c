#include <stdio.h>
#include <string.h>

int main (void)
{
	// These are constant expressions so expect them to get optimized away

	if (strlen ("Testing") != 7)
		return 1;

	if (strlen ("") != 0)
		return 2;

	printf("Test passed.\n");
	return 0;
}

