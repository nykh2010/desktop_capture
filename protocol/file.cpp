#include "file.h"
#include "media.h"

int unmux_file(file_t *unpackfile, FILE *file) {
	return 0;
}

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
error:
	free(unpack_file);
	return NULL;
}