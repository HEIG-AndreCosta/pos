
#ifndef ACCEL_H
#define ACCEL_H

#include <stdint.h>
typedef enum {
	FULL_SCALE_2G = 0,
	FULL_SCALE_4G = 2,
	FULL_SCALE_8G = 3,
	FULL_SCALE_16G = 1,
} accel_scale_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
	accel_scale_t scale;
} accel_read_t;

#endif
