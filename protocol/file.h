#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint8_t * head;
	uint16_t length;
} head_t;

typedef struct frame_t {
	uint8_t *frame;
	uint16_t length;
	struct frame_t *next_frame;
} frame_t;

typedef struct {
	head_t *head;
	frame_t *frame;
} file_t;

file_t *file_unpack(FILE *file);