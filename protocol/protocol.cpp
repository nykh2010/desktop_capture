#include "protocol.h"
#include "device.h"
#include <stdlib.h>
#include <string.h>

#define MAX_BUFF				4096
#define	START_SIZE				2
#define INFOCODE_SIZE			1
#define	CURFRAME_SIZE			2
#define	TOTALFRAME_SIZE			2
#define INFOSEG_LEN_SIZE		2
#define END_SIZE				2

#define MAX_PAYLOAD_LENGTH		(MAX_BUFF-START_SIZE-INFOCODE_SIZE-TOTALFRAME_SIZE-INFOSEG_LEN_SIZE-END_SIZE)

#define HEAD		0x00
#define VIDEO		0x01
#define AUDIO		0x02

typedef struct {
	uint16_t length;
	uint8_t *data;
} infoseg_t;

#define START_POS			0
#define	INFOCODE_POS		2
#define	CURFRAME_POS		3
#define	TOTALFRAME_POS		5
#define	INFOSEG_LEN_POS		7
#define INFOSEG_DATA_POS	9
typedef struct {
	uint8_t buff[4096];
	uint16_t buff_pos;
	uint16_t start;
	uint8_t info_code;
	uint16_t cur_frame;
	uint16_t total_frame;
	uint16_t infoseg_len;
	uint8_t *infoseg_data;
	uint16_t end;
} packet_t;

packet_t * create_packet(uint8_t info_code, uint16_t total_frame, infoseg_t * info_seg);
int send_packet(uint16_t cur_frame, packet_t *packet);
int send_frame(frame_t *frame);

int send_packet(uint16_t cur_frame, packet_t *packet) {
	uint16_t buff_offset, size;
	memcpy(packet->buff+START_POS, &packet->start, START_SIZE);
	memcpy(packet->buff+INFOCODE_POS, &packet->info_code, INFOCODE_SIZE);
	memcpy(packet->buff + CURFRAME_POS, &cur_frame, CURFRAME_SIZE);
	memcpy(packet->buff + TOTALFRAME_POS, &packet->total_frame, TOTALFRAME_SIZE);
	buff_offset = cur_frame * MAX_PAYLOAD_LENGTH;
	size = (packet->infoseg_len - buff_offset) > MAX_PAYLOAD_LENGTH ? MAX_PAYLOAD_LENGTH : (packet->infoseg_len - buff_offset);
	memcpy(packet->buff + INFOSEG_DATA_POS, packet->infoseg_data + buff_offset, size);
	memcpy(packet->buff + INFOSEG_DATA_POS + size, &packet->end, END_SIZE);
	
	return device_send(packet->buff, INFOSEG_DATA_POS + size + END_SIZE);
}

packet_t * create_packet(uint8_t info_code, uint16_t total_frame, infoseg_t * info_seg) {
	packet_t *packet = (packet_t *)calloc(1, sizeof(packet_t));
	if (packet == NULL) {
		return NULL;
	}
	packet->start = 0xFFFF;
	packet->info_code = info_code;
	packet->total_frame = total_frame;
	packet->infoseg_len = info_seg->length;
	packet->infoseg_data = info_seg->data;
	packet->end = 0x0a0d;

	packet->buff_pos = 0;
	return packet;
}

int send_head(head_t *head) {
	uint16_t total_frame;
	uint16_t cur_frame;
	uint8_t *buf;
	uint16_t size;
	total_frame = head->length / MAX_BUFF;
	if (head->length % MAX_BUFF) {
		total_frame += 1;
	}
	for (cur_frame = 0; cur_frame < total_frame; cur_frame++) {
		buf = head->head + cur_frame * MAX_BUFF;
		size = (cur_frame == total_frame - 1)?(MAX_BUFF):(head->length % MAX_BUFF);
		device_send(buf,size);
	}
	return 0;
#if 0
	uint16_t total_frame;
	uint16_t cur_frame;
	int status,err_flag=0;
	total_frame = head->length / MAX_PAYLOAD_LENGTH;
	if (head->length % MAX_PAYLOAD_LENGTH) {
		total_frame += 1;
	}
	infoseg_t *info_seg = (infoseg_t *)calloc(1, sizeof(info_seg));
	if (info_seg == NULL) {
		return -1;
	}
	info_seg->length = head->length;
	info_seg->data = head->head;
	packet_t *packet = create_packet(HEAD, total_frame, info_seg);
	if (packet == NULL) {
		goto error;
	}
	for (cur_frame = 0; cur_frame < total_frame; cur_frame++) {
		status = send_packet(cur_frame, packet);
		if (status < 0) {
			printf("send_packet error\n");
			free(packet);
			goto error;
		}
	}
	free(packet);
	free(info_seg);
	return 0;
error:
	free(info_seg);
	return -1;
#endif
}

int send_frame(frame_t *frame) {
	uint16_t total_frame;
	uint16_t cur_frame;
	uint8_t *buf;
	int size;
	total_frame = frame->length / MAX_BUFF;
	if (frame->length % MAX_BUFF) {
		total_frame += 1;
	}
	for (cur_frame = 0; cur_frame < total_frame; cur_frame++) {
		buf = frame->frame + cur_frame * MAX_BUFF;
		//size = (cur_frame != total_frame - 1)?(MAX_BUFF):(frame->length % MAX_BUFF);
		device_send(buf, MAX_BUFF);
	}
	return 0;
#if 0
	uint16_t cur_frame=0, total_frame=0;
	int status;
	frame_t *curFrame = frame;
	infoseg_t *info_seg = (infoseg_t *)calloc(1, sizeof(info_seg));
	if (info_seg == NULL) {
		return -1;
	}
	while (curFrame != NULL) {
		//send packet
		total_frame = 0;
		total_frame = curFrame->length / MAX_PAYLOAD_LENGTH;
		if (curFrame->length % MAX_PAYLOAD_LENGTH) {
			total_frame += 1;
		}
		info_seg->length = curFrame->length;
		info_seg->data = curFrame->frame;
		packet_t *packet = create_packet(VIDEO, total_frame, info_seg);
		if (packet == NULL) {
			goto error;
		}
		for (cur_frame = 0; cur_frame < total_frame; cur_frame++) {
			status = send_packet(cur_frame, packet);
			if (status < 0) {
				printf("send_packet error\n");
				free(packet);
				goto error;
			}
		}
		free(packet);
		curFrame = curFrame->next_frame;
	}
	free(info_seg);
	return 0;
error:
	free(info_seg);
	return -1;
#endif
}
