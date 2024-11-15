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
		printf("New event. Type: %d Code: %d Value: %d\n", event.type,
		       event.code, event.value);
	}
	return EXIT_SUCCESS;
}
