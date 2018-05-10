#pragma once
#include <stdint.h>
typedef struct PACKET {
	uint8_t *data;
	int size;
	struct PACKET *next;
} packet_t;

typedef struct {
	packet_t *packet_list;
	packet_t *packet_head;
	packet_t *packet_tail;
} desktop_capture_t;

desktop_capture_t *open_capture(const char *path);
packet_t *read_packet();
int release_packet(packet_t *packet);
int close_capture(desktop_capture_t *capture);