#include <stdlib.h>
#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	struct input_event event;

	if (argc < 2) {
		printf("Usage %s <event_file>", argv[0]);
		return EXIT_FAILURE;
	}
	int fd = open(argv[1], O_RDWR);

	while (1) {
		if (read(fd, &event, sizeof(event)) == -1) {
			break;
		}
		if (event.type == EV_KEY && event.value == 1) {
			printf("New event. Type: %d Code: %d Value: %d ",
			       event.type, event.code, event.value);
			switch (event.code) {
			case KEY_ENTER:
				printf("KEY_ENTER\n");
				break;
			case KEY_UP:
				printf("KEY_UP\n");
				break;
			case KEY_DOWN:
				printf("KEY_DOWN\n");
				break;
			case KEY_LEFT:
				printf("KEY_LEFT\n");
				break;
			case KEY_RIGHT:
				printf("KEY_RIGHT\n");
				break;
			}
		}
	}
	return EXIT_SUCCESS;
}
