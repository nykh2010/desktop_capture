#include "desktop.h"

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libavdevice\avdevice.h>
};

#include <Windows.h>

desktop_capture_t *desktop_capture = NULL;
AVFormatContext *pDskFormatCtx = NULL;
AVFormatContext *pTsFormatCtx = NULL;
AVCodecContext *pCodecCtx = NULL;
AVCodec *pCodec = NULL;
AVFrame *pFrame = NULL, *pFrameYUV = NULL;
struct SwsContext *img_convert_ctx = NULL;

HANDLE read_write_lock;

static void print_errmsg(int errnum) {
	char errmsg[64] = { 0 };
	av_make_error_string(errmsg, sizeof(errmsg), errnum);
	printf("%s\n", errmsg);
}

DWORD WINAPI capture_handle(LPVOID pm) {
	int status,got_picture,result;
	while (1) {
		AVPacket dsk_packet;
		status = av_read_frame(pDskFormatCtx,&dsk_packet);
		if (status < 0) {
			print_errmsg(status);
			continue;
		}
		status = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &dsk_packet);
		if (status < 0) {
			continue;
		}
		if (got_picture) {
			pFrameYUV->pts = pFrame->pts;
			pFrameYUV->pkt_pts = pFrame->pkt_pts;
			pFrameYUV->pkt_dts = pFrame->pkt_dts;
			sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
			AVPacket outPacket;
			int size = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
			av_new_packet(&outPacket, size);
			status = avcodec_encode_video2(pTsFormatCtx->streams[0]->codec, &outPacket, pFrameYUV, &result);
			if (status < 0) {
				continue;
			}
			if (result) {
				outPacket.pts = av_rescale_q(pFrameYUV->pts, pDskFormatCtx->streams[0]->time_base, pTsFormatCtx->streams[0]->time_base);
				outPacket.dts = av_rescale_q(pFrameYUV->pkt_dts, pDskFormatCtx->streams[0]->time_base, pTsFormatCtx->streams[0]->time_base);
				status = av_interleaved_write_frame(pTsFormatCtx, &outPacket);
				if (status < 0) {
					continue;
				}
			}
			av_free_packet(&outPacket);
		}
		av_free_packet(&dsk_packet);
	}
	av_write_trailer(pTsFormatCtx);
	sws_freeContext(img_convert_ctx);
	avcodec_close(pCodecCtx);
	av_free(pFrame);
	av_free(pFrameYUV);
	avformat_close_input(&pDskFormatCtx);
	ExitThread(-1);
}

int packet_append(packet_t *packet) {
	WaitForSingleObject(read_write_lock, 200);
	if (desktop_capture->packet_tail->next == NULL) {
		desktop_capture->packet_tail->next = packet;
		desktop_capture->packet_tail = packet;
	}
	ReleaseMutex(read_write_lock);
	return 0;
}

static int write_ts_frame(void *opaque, uint8_t *buf, int buf_size) {
	packet_t *packet = (packet_t *)calloc(1, sizeof(packet_t));
	if (packet == NULL) {
		return -1;
	}
	packet->data = (uint8_t *)calloc(1, buf_size);
	if (packet->data == NULL) {
		return -1;
	}
	packet->size = buf_size;
	memcpy(packet->data, buf, buf_size);
	packet->next = NULL;
	packet_append(packet);
	return 0;
}

desktop_capture_t *open_capture(const char *path) {
	int i, videoindex = -1;
	int status;
	//0、初始化desktop_capture
	desktop_capture = (desktop_capture_t *)calloc(1, sizeof(desktop_capture_t));
	if (desktop_capture == NULL) {
		return NULL;
	}
	desktop_capture->packet_head = (packet_t *)calloc(1,sizeof(packet_t));
	desktop_capture->packet_tail = desktop_capture->packet_head;

	read_write_lock = CreateMutex(NULL, TRUE, "packet list");

	av_register_all();
	avdevice_register_all();
	//1、配置输入源
	pDskFormatCtx = avformat_alloc_context();
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
	pCodecCtx = pDskFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		return NULL;
	}
	status = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (status < 0) {
		return NULL;
	}
	//4、配置输出源
	AVCodecID ts_videocodecid = AV_CODEC_ID_H264;		//以H.264进行编码
	AVCodec *pH264Codec = avcodec_find_encoder(ts_videocodecid);
	AVCodecContext *pH264CodecCtx = avcodec_alloc_context3(pH264Codec);
	pH264CodecCtx->width = pCodecCtx->width;
	pH264CodecCtx->height = pCodecCtx->height;
	AVRational framerate = { 1, 30 };
	pH264CodecCtx->time_base = framerate;
	pH264CodecCtx->gop_size = 4;
	pH264CodecCtx->max_b_frames = 2;
	pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	av_opt_set(pH264CodecCtx->priv_data, "preset", "ultrafast", 0);
	av_opt_set(pH264CodecCtx->priv_data, "tune", "zerolatency", 0);
	pH264CodecCtx->qmin = 10;
	pH264CodecCtx->qmax = 51;
	status = avcodec_open2(pH264CodecCtx, pH264Codec, NULL);				//打开编码器
	if (status < 0) {
		return NULL;
	}
	avformat_alloc_output_context2(&pTsFormatCtx, NULL, NULL, "test.ts");		//以TS进行封装
	avio_open2(&pTsFormatCtx->pb, "test.ts", AVIO_FLAG_WRITE, NULL, NULL);
	pTsFormatCtx->pb->write_packet = write_ts_frame;		//自定写封装函数
	AVStream *tsStream = avformat_new_stream(pTsFormatCtx, pH264Codec);			//创建视频流
	tsStream->codec = pH264CodecCtx;
	tsStream->time_base = tsStream->codec->time_base;
	//5、准备
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	uint8_t *data = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, data, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	pFrameYUV->width = pCodecCtx->width;
	pFrameYUV->height = pCodecCtx->height;
	pFrameYUV->format = AV_PIX_FMT_YUV420P;
	//6、准备格式转换
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pFrameYUV->width, pFrameYUV->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	//7、尝试写入文件头
	status = avformat_write_header(pTsFormatCtx, NULL);
	if (status < 0) {
		return NULL;
	}
	//多线程抓取桌面图像
	HANDLE capture_thread = CreateThread(NULL, 0, capture_handle, NULL, 0, NULL);
	if (capture_thread == NULL) {
		return NULL;
	}
	return desktop_capture;
}

packet_t *read_packet() {
	return desktop_capture->packet_head->next;
}

int release_packet(packet_t *packet) {
	WaitForSingleObject(read_write_lock, 200);
	desktop_capture->packet_head->next = packet->next;
	if (desktop_capture->packet_head->next == NULL) {
		desktop_capture->packet_tail = desktop_capture->packet_head;
	}
	ReleaseMutex(read_write_lock);
	free(packet->data);
	free(packet);
	return 0;
}

int close_capture(desktop_capture_t *capture) {
	packet_t *packet1, *packet2;
	packet1 = capture->packet_list;
	packet2 = capture->packet_list->next;
	while (packet1->next != NULL) {
		packet2 = packet1->next;
		free(packet1->data);
		free(packet1);
		packet1 = packet2;
	}
	free(packet1->data);
	free(packet1);
	free(capture);
	return 0;
}