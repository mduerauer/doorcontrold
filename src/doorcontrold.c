/**
 * doorcontrold.c
 * --------------
 *
 * author: Markus Dürauer
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pifacedigital.h"

int main (int argc, char *argv[])
{
	int hw_addr = 0;

	printf("Opening piface digital connection at location %d\n", hw_addr);
	pifacedigital_open(hw_addr);

	printf("Closing piface digital connection at location %d\n", hw_addr);
    	pifacedigital_close(hw_addr);

	return 0;
}
