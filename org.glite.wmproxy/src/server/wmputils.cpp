#include "wmputils.h"

#include <iostream>

using namespace std;
	
static void
waitForAWhile(int seconds)
{
	fprintf(stderr, "----- Waiting for a while -----\n");
	time_t startTime = time(NULL);
	time_t endTime = time(NULL);
	while((endTime - startTime) < seconds) {
		endTime = time(NULL);
	}
	fprintf(stderr, "----- End waiting -----\n");
}


