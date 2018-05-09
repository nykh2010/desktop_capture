#include "desktop.h"

extern "C" {
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libavdevice\avdevice.h>
}

int open_gdigrab() {
	
}

static int write_ts_frame(void *opaque, uint8_t *buf, int buf_size) {

}

desktop_capture_t *open_capture(const char *path) {
	int i, videoindex = -1;
	int status;
	av_register_all();
	avdevice_register_all();
	//1����������Դ
	AVFormatContext *pDskFormatCtx = avformat_alloc_context();
	AVDictionary *options = NULL;
	av_dict_set(&options, "framerate", "15", 0);
	AVInputFormat *pDskInFormat = av_find_input_format("gdigrab");
	avformat_open_input(&pDskFormatCtx, "desktop", pDskInFormat, &options);
	//2��������Ƶ��
	if (avformat_find_stream_info(pDskFormatCtx, NULL) < 0) {
		return NULL;
	}
	for (i = 0; i < pDskFormatCtx->nb_streams; i++) {
		if (pDskFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	}
	if (videoindex < 0) {
		return NULL;
	}
	//3����ȡ��Ƶ������������
	AVCodecContext *pCodecCtx = pDskFormatCtx->streams[videoindex]->codec;
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		return NULL;
	}
	status = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (status < 0) {
		return NULL;
	}
	//4���������Դ
	AVFormatContext *pTsFormatCtx = NULL;
	AVCodecID ts_videocodecid = AV_CODEC_ID_H264;		//��H.264���б���
	AVCodec *pH264Codec = avcodec_find_encoder(ts_videocodecid);
	avformat_alloc_output_context2(&pTsFormatCtx, NULL, NULL, "test.ts");		//��TS���з�װ
	avio_open2(&pTsFormatCtx->pb, "test.ts", AVIO_FLAG_WRITE, NULL, NULL);
	pTsFormatCtx->pb->write_packet = write_ts_frame;		//�Զ�д��װ����
	AVStream *tsStream = avformat_new_stream(pTsFormatCtx, pH264Codec);			//������Ƶ��
	//5��׼��

}
packet_t *read_packet();
int close_capture(desktop_capture_t *capture);