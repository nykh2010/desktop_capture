#pragma once

typedef struct packet_t {
	uint8_t *data;
	int size;
	struct packet_t *next;
} packet_t;

typedef struct {
	packet_t *packet_list;
	packet_t *packet_head;
	packet_t *packet_tail;
} desktop_capture_t;

desktop_capture_t *open_capture(const char *path);
packet_t *read_packet();
int close_capture(desktop_capture_t *capture);