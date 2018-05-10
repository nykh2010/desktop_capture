#include <stdio.h>
#include <stdlib.h>
#include "device.h"
#include "protocol.h"
#include "file.h"
#include "status.h"

#define FILE_PATH		"desktop.ts"

int main(int argc, char *argv[]) {
	int status, flag=1;
	file_t *source = NULL;
	device = device_init(MASTER, 7);
	if (device == NULL) {
		printf("device_init error\n");
		return -1;
	}
	source = open_source(FILE_PATH);
	if (source == NULL) {
		printf("open_file error\n");
		return -1;
	}
	do {
		status = send_head(source->head);
	} while (status != SUCCESS);
	while (flag) {
		if (read_source(source) < 0)
			continue;
		status = send_frame(source->frame);
		if (status < 0) {
			flag = 0;
		}
	}
	close_source(source);
	device_release(device);
	return 0;
}
