#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <fcntl.h>
#include <regex.h>
#include <signal.h>
#include <sys/time.h>

#include "list.h"
#include "cam.h"
#include "main.h"

#define DOCS_LOCATION_IMAGES	"/root/ramdisk/images"
#define DOCS_LOCATION_THUMBS	"thumbs"
#define DOCS_IMAGE_SIZE		320

double makeTimestamp(int tv_sec, int tv_usec) {
	double result;

	result  = ((double) tv_usec) / 1000000;
	result += tv_sec;

	return result;
}

double getTimestamp() {
	struct timeval  my_timeval;
	int ret;

	ret = gettimeofday(&my_timeval, NULL);

	if( ret < 0 ) {
		perror("ERROR: ");
		return -1;
	}

	return makeTimestamp(my_timeval.tv_sec, my_timeval.tv_usec);
}

int main() {
	char path[128];
	RUNNING = 1;

	// Create directories
	mkdir(DOCS_LOCATION_IMAGES, 0777);
	snprintf(path, 128, "%s/%s", DOCS_LOCATION_IMAGES, DOCS_LOCATION_THUMBS);
	mkdir(path, 0777);

	// Start the cam loop. This may as well exit the process if no cam is found
	camLoop(DOCS_LOCATION_IMAGES, DOCS_LOCATION_THUMBS, DOCS_IMAGE_SIZE);

	return 0;
}
