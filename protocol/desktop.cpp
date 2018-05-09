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
	//1、配置输入源
	AVFormatContext *pDskFormatCtx = avformat_alloc_context();
	AVDictionary *options = NULL;
	av_dict_set(&options, "framerate", "15", 0);
	AVInputFormat *pDskInFormat = av_find_input_format("gdigrab");
	avformat_open_input(&pDskFormatCtx, "desktop", pDskInFormat, &options);
	//2、搜索视频流
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
	//3、获取视频流编码器并打开
	AVCodecContext *pCodecCtx = pDskFormatCtx->streams[videoindex]->codec;
	AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		return NULL;
	}
	status = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (status < 0) {
		return NULL;
	}
	//4、配置输出源
	AVFormatContext *pTsFormatCtx = NULL;
	AVCodecID ts_videocodecid = AV_CODEC_ID_H264;		//以H.264进行编码
	AVCodec *pH264Codec = avcodec_find_encoder(ts_videocodecid);
	avformat_alloc_output_context2(&pTsFormatCtx, NULL, NULL, "test.ts");		//以TS进行封装
	avio_open2(&pTsFormatCtx->pb, "test.ts", AVIO_FLAG_WRITE, NULL, NULL);
	pTsFormatCtx->pb->write_packet = write_ts_frame;		//自定写封装函数
	AVStream *tsStream = avformat_new_stream(pTsFormatCtx, pH264Codec);			//创建视频流
	//5、准备

}
packet_t *read_packet();
int close_capture(desktop_capture_t *capture);