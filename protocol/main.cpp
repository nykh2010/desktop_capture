#include <stdio.h>
#include <stdlib.h>
#include "device.h"
#include "protocol.h"
#include "file.h"
#include "status.h"

#define FILE_PATH		"outfile.mp4"

FILE *source = NULL;

FILE *open_file(const char *path);
int close_file(FILE *file);

int main(int argc, char *argv[]) {
	int status, flag=1;

	device = device_init(MASTER, 7);
	if (device == NULL) {
		printf("device_init error\n");
		return -1;
	}
	source = open_file(FILE_PATH);
	if (source == NULL) {
		printf("open_file error\n");
		return -1;
	}
	file_t *file = file_unpack(source);
	do {
		status = send_head(file->head);
	} while (status != SUCCESS);
	while (flag) {
		status = send_frame(file->frame);
		if (status < 0) {
			flag = 0;
		}
	}
	close_file(source);
	device_release(device);
	return 0;
}

FILE *open_file(const char *path) {
	return fopen(path, "rb+");
}

int close_file(FILE *file) {
	if (file == NULL) {
		return 0;
	}
	return fclose(file);
}