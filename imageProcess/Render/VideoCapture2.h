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

		void Init(int width, int height, int fpsrate, int bitrate, std::string outFilePath);//视频文件的宽、高、帧率、比特率及输出路径
		void AddFrame(uint8_t *data, int srcWidth, int srcHeight);//输入图像的数据、宽、高
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
		std::string _outFilePath;   //输出视频的路径
		std::string _tempFilePath;  //临时输出视频的路径，后缀名为h264（先生成h264的临时视频，再转为目标类型的视频）
		bool _needRotate;//png序列帧转视频需要将图片翻转180°，jpg序列帧转视频不需要将图片翻转

		void Free();
		void Remux();
		AVFrame* RotateRGBOrigData(uint8_t *data, int width, int height);
	};
