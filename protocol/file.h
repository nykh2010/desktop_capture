#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint8_t * head;
	int length;
} head_t;

typedef struct frame_t {
	uint8_t *frame;
	int length;
} frame_t;

typedef struct {
	head_t *head;
	frame_t *frame;
} file_t;

file_t *open_source(const char *path);
int read_source(file_t *source);
int close_source(file_t *source);
