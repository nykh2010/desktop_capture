#pragma once

#include <stdint.h>

#define MASTER		1
#define SLAVE		2

typedef struct {
	uint8_t mode;
	uint8_t speed;
	uint8_t buff[4096];
} device_t;

extern device_t *device;

device_t *device_init(uint8_t mode, uint8_t speed);
int device_send(uint8_t *buff, uint16_t length);
int device_receive(uint8_t *buff, uint16_t *length);
int device_release(device_t *device);