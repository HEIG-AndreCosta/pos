#include "accel.h"
#include <fcntl.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define NO_LEDS	  5
#define BASE_PATH "/sys/class/leds/mydev_led"
#define FILE_NAME "brightness"

static int led_fd[NO_LEDS];
static bool led_status[NO_LEDS];
ssize_t led_set(int index, const char *buf)
{
	return write(led_fd[index], buf, strlen(buf));
}
void led_update(int index, bool value, bool is_reversed)
{
	// if we're reversed, we need to toggle the led
	if (is_reversed && value) {
		value = !led_status[index];
	}

	led_status[index] = value;
	if (value) {
		led_set(index, "1");

	} else {
		led_set(index, "0");
	}
}

int main(int argc, char **argv)
{
	char file_path[50];

	if (argc < 2) {
		printf("Usage %s <accel_dev_file>", argv[0]);
		return EXIT_FAILURE;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("Couldn't open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	for (size_t i = 0; i < NO_LEDS; ++i) {
		sprintf(file_path, BASE_PATH "%zu/" FILE_NAME, i);
		led_fd[i] = open(file_path, O_RDWR);
		if (led_fd[i] < 0) {
			printf("Couldn't open file %s\n", file_path);
			return EXIT_FAILURE;
		}
	}

	accel_read_t data;
	while (1) {
		if (read(fd, &data, sizeof(data)) == -1) {
			break;
		}
		double roll = atan2(data.y, data.z) * 180 / M_PI;
		double pitch = atan2(-data.x,
				     sqrt(data.y * data.y + data.z * data.z)) *
			       180 / M_PI;
		const bool is_flat = roll > -10 && roll < 10 && pitch > -10 &&
				     pitch < 10;

		const bool is_reversed = data.z < 0;
		led_update(0, is_flat, is_reversed);
		led_update(1, pitch > 10, is_reversed);
		led_update(2, roll < -10, is_reversed);
		led_update(3, pitch < -10, is_reversed);
		led_update(4, roll > 10, is_reversed);

		printf("Temperature : %.2f°C\n",
		       (double)data.raw_temp / 16 + 25);
		//Wait 100 ms
		usleep(100000);
	}

	return 0;
}
