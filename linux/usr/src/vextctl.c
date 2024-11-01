#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NO_LEDS	  5
#define BASE_PATH "/sys/class/leds/mydev_led"
#define FILE_NAME "brightness"

int main(int argc, char **argv)
{
	FILE *fps[NO_LEDS];
	char file_path[50];
	for (size_t i = 0; i < NO_LEDS; ++i) {
		sprintf(file_path, BASE_PATH "%zu/" FILE_NAME, i);
		fps[i] = fopen(file_path, "r+");
		if (!fps[i]) {
			printf("Couldn't open file %s\n", file_path);
			return EXIT_FAILURE;
		}
	}

	fputc('1', fps[0]);
	fseek(fps[0], SEEK_SET, 0);
	size_t start = 1;
	while (1) {
		for (size_t i = start; i < NO_LEDS; i += 2) {
			fseek(fps[i], SEEK_SET, 0);
			fputc('1', fps[i]);
		}
		usleep(500000);
		for (size_t i = start; i < NO_LEDS; i += 2) {
			fseek(fps[i], SEEK_SET, 0);
			fputc('0', fps[i]);
		}
		if (start == 1) {
			start = 2;
		} else {
			start = 1;
		}
	}

	return 0;
}
