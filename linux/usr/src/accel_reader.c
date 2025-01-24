

#include "accel.h"
#include <math.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage %s <accel_dev_file>", argv[0]);
		return EXIT_FAILURE;
	}
	int fd = open(argv[1], O_RDWR);
	accel_read_t data;
	while (1) {
		if (read(fd, &data, sizeof(data)) == -1) {
			break;
		}
		uint16_t roll = atan2(data.y, data.z);
		uint16_t pitch =
			atan2(-data.x, sqrt(data.y * data.y + data.z * data.z));

		printf("Roll :%d Pitch %d\n", roll, pitch);
		sleep(1);
	}
	return EXIT_SUCCESS;
}
