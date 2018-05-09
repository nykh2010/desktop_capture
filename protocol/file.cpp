#include "file.h"
#include "media.h"
#include "desktop.h"
#include <stdlib.h>

desktop_capture_t *capture = NULL;
file_t *source = NULL;

file_t *file_unpack(FILE *file) {	
	int status;
	file_t *unpack_file = (file_t *)calloc(1, sizeof(file_t));
	if (unpack_file == NULL) {
		return NULL;
	}
	status = unmux_file(unpack_file, file);
	if (status < 0) {
		goto error;
	}
	return unpack_file;
error:
	free(unpack_file);
	return NULL;
}


file_t *open_source(const char *path) {
	source = (file_t *)calloc(1, sizeof(file_t));
	if (source == NULL) {
		return NULL;
	}
	capture = open_capture(path);
	if (capture == NULL) {
		goto error;
	}
	while (capture->packet_head->next == NULL);		//等待屏幕捕获开始
	source->head->head = capture->packet_head->next->data;
	source->head->length = capture->packet_head->next->size;
	return source;
error:
	free(source);
	return NULL;
}

int read_source(file_t *source) {
	packet_t *packet = read_packet();
	if (packet == NULL) {
		return -1;
	}
	source->frame->frame = packet->data;
	source->frame->length = packet->size;
	return 0;
}

int close_source(file_t *source) {
	if (source == NULL) {
		return -1;
	}
	close_capture(capture);
	free(source);
	return 0;
}