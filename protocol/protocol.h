#pragma once
#include <stdint.h>
#include "file.h"

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

int send_head(head_t *head);
int send_frame(frame_t *frame);

