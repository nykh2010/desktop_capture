#include "media.h"
#include "desktop.h"

int unmux_file(file_t *packet, FILE *in_file) {
	return unmux_desktopfile(packet, in_file);
}