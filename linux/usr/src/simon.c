#include <linux/input-event-codes.h>
#include <stdlib.h>
#include <linux/input.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <assert.h>
#define NO_LEDS	  5
#define BASE_PATH "/sys/class/leds/mydev_led"
#define FILE_NAME "brightness"

const static int keys[] = { KEY_ENTER, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN };
static int get_key_pressed(int event_fd);

int get_key_pressed(int event_fd)
{
	struct input_event event;
	int result = -1;

	if (read(event_fd, &event, sizeof(event)) == -1) {
		printf("Failed to read event file");
		exit(EXIT_FAILURE);
	}
	assert(event.type == EV_KEY && event.value == 1);
	result = event.code;

	//Flush release event
	if (read(event_fd, &event, sizeof(event)) == -1) {
		printf("Failed to read event file");
		exit(EXIT_FAILURE);
	}
	assert(event.type == EV_KEY && event.code == result &&
	       event.value == 0);
	//Flush type == 0 event
	if (read(event_fd, &event, sizeof(event)) == -1) {
		printf("Failed to read event file");
		exit(EXIT_FAILURE);
	}
	assert(event.type == 0 && event.code == 0 && event.value == 0);
	return result;
}
int main(int argc, char **argv)
{
	int led_fds[NO_LEDS];
	int event_fd;
	char file_path[50];
	if (argc < 2) {
		printf("Usage %s <event_file_path>", argv[0]);
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < NO_LEDS; ++i) {
		sprintf(file_path, BASE_PATH "%zu/" FILE_NAME, i);
		led_fds[i] = open(file_path, O_RDWR);
		if (led_fds[i] == -1) {
			printf("Couldn't open file %s\n", file_path);
			return EXIT_FAILURE;
		}
	}

	event_fd = open(argv[1], O_RDONLY);

	printf("Welcome to Simon!\n");

	for (size_t i = 0; i < NO_LEDS; ++i) {
		write(led_fds[i], "1", sizeof("1"));
	}
	usleep(1000000);
	for (size_t i = 0; i < NO_LEDS; ++i) {
		write(led_fds[i], "0", sizeof("1"));
	}

	size_t round = 0;
	size_t current_size = 10;
	int *sequence = malloc(current_size * sizeof(int));
	assert(sequence);
	srand(time(NULL));
	while (1) {
		if (round == current_size) {
			current_size *= 2;
			sequence = realloc(sequence, current_size);
			assert(sequence);
		}

		sequence[round++] = (rand() % (NO_LEDS - 1)) + 1;

		for (int i = 0; i < round; ++i) {
			const int curr_led = sequence[i];
			usleep(500000);
			write(led_fds[curr_led], "1", sizeof("1"));
			usleep(500000);
			write(led_fds[curr_led], "0", sizeof("1"));
		}
		usleep(200000);
		write(led_fds[0], "1", sizeof("1"));
		usleep(500000);
		write(led_fds[0], "0", sizeof("1"));
		for (int i = 0; i < round; ++i) {
			const int curr_key = keys[sequence[i]];
			const int key_pressed = get_key_pressed(event_fd);
			if (key_pressed == curr_key) {
				write(led_fds[curr_key], "1", sizeof("1"));
				usleep(100000);
				write(led_fds[curr_key], "0", sizeof("1"));
			} else {
				printf("Game Over. You made it to round %zu\n",
				       round);
				goto end;
			}
		}
	}
end:
	free(sequence);
	return 0;
}
