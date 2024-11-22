#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define NO_LEDS	  5
#define BASE_PATH "/dev/vextled"

int main(int argc, char **argv)
{
	int fds[NO_LEDS];
	char file_path[50];
	for (size_t i = 0; i < NO_LEDS; ++i) {
		sprintf(file_path, BASE_PATH "%zu", i);
		printf("Opening %s\n", file_path);
		fds[i] = open(file_path, O_RDWR);
		if (fds[i] < 0) {
			printf("Couldn't open file %s\n", file_path);
			return EXIT_FAILURE;
		}
	}

	printf("Starting Sequence\n");
	write(fds[0], "1", sizeof("1"));
	size_t start = 1;
	while (1) {
		for (size_t i = start; i < NO_LEDS; i += 2) {
			write(fds[i], "1", sizeof("1"));
		}
		usleep(500000);
		for (size_t i = start; i < NO_LEDS; i += 2) {
			write(fds[i], "0", sizeof("0"));
		}
		if (start == 1) {
			start = 2;
		} else {
			start = 1;
		}
	}

	return 0;
}
