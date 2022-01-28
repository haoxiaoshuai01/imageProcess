//#define VIDEOCAPTURE_API __declspec(dllexport)//[YHQ]

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <string> 

extern "C"
{

#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/time.h>
#include <libavutil/audio_fifo.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}


void avlog_cb(void *, int level, const char * fmt, va_list vargs);

	class VideoCapture2 {
	public:

		VideoCapture2() {
			_oformat = NULL;
			_ofctx = NULL;
			_videoStream = NULL;
			_videoFrame = NULL;
			_swsCtx = NULL;
			_frameCounter = 0;
			_outFilePath = "";
			_tempFilePath = "";
			_needRotate = true;

			// Initialize libavcodec
			av_register_all();
			av_log_set_callback(avlog_cb);
		}

		~VideoCapture2() {
			Free();
		}

		void Init(int width, int height, int fpsrate, int bitrate, std::string outFilePath);//��Ƶ�ļ��Ŀ��ߡ�֡�ʡ������ʼ����·��
		void AddFrame(uint8_t *data, int srcWidth, int srcHeight);//����ͼ������ݡ�����
        //void AddFrame(beResource::Image* srcImg);
		void Finish();

	private:
        AVOutputFormat *_oformat= nullptr;
        AVFormatContext *_ofctx = nullptr;
        AVStream *_videoStream= nullptr;
        AVFrame *_videoFrame= nullptr;
        AVCodec *_codec= nullptr;
        AVCodecContext *_cctx= nullptr;
        SwsContext *_swsCtx= nullptr;
		int _frameCounter;
		int _fps;
		std::string _outFilePath;   //�����Ƶ��·��
		std::string _tempFilePath;  //��ʱ�����Ƶ��·������׺��Ϊh264��������h264����ʱ��Ƶ����תΪĿ�����͵���Ƶ��
		bool _needRotate;//png����֡ת��Ƶ��Ҫ��ͼƬ��ת180�㣬jpg����֡ת��Ƶ����Ҫ��ͼƬ��ת

		void Free();
		void Remux();
		AVFrame* RotateRGBOrigData(uint8_t *data, int width, int height);
	};
