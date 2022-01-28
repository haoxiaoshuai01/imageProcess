#include "VideoCapture2.h"
#include <cstdio>

//#define VIDEO_TMP_FILE "d:\\tmp.h264"
//#define FINAL_FILE_NAME "d:\\record.mp4"

#include "QDebug"
using namespace std;


std::ofstream logFile;

void Log(std::string str) {
	logFile.open("Logs.txt", std::ofstream::app);
	logFile.write(str.c_str(), str.size());
	logFile.close();
}

typedef void(*FuncPtr)(const char *);
FuncPtr ExtDebug;
char errbuf[32];

void Debug(std::string str, int err) {
    std::cout << (str + " " + std::to_string(err));
	if (err < 0) {
		av_strerror(err, errbuf, sizeof(errbuf));
		str += errbuf;
	}
     std::cout << (str);
    //ExtDebug(str.c_str());
}

void avlog_cb(void *, int level, const char * fmt, va_list vargs) {
	static char message[8192];
	vsnprintf_s(message, sizeof(message), fmt, vargs);
	Log(message);
}

void VideoCapture2::Init(int width, int height, int fpsrate, int bitrate, std::string outFilePath) 
{
	//根据后缀名设置视频比特率
	//int bitrate = -1;
	//string suffixStr = _outFilePath.substr(_outFilePath.find_last_of('.') + 1);//获取文件后缀
	//if (suffixStr == "avi")
	//{
	//	bitrate = 210;
	//}
	//else//mp4格式
	//{
	//	bitrate = 21000;
	//}

	_outFilePath = outFilePath;
	_tempFilePath = _outFilePath.substr(0, _outFilePath.rfind(".")) + ".h264";
	
	_fps = fpsrate;

	int err;

	if (!(_oformat = av_guess_format(NULL, _tempFilePath.c_str(), NULL))) {
		Debug("Failed to define output format", 0);
		return;
	}

	if ((err = avformat_alloc_output_context2(&_ofctx, _oformat, NULL, _tempFilePath.c_str()) < 0)) {
		Debug("Failed to allocate output context", err);
		Free();
		return;
	}

	if (!(_codec = avcodec_find_encoder(_oformat->video_codec))) {
		Debug("Failed to find encoder", 0);
		Free();
		return;
	}

	if (!(_videoStream = avformat_new_stream(_ofctx, _codec))) {
		Debug("Failed to create new stream", 0);
		Free();
		return;
	}

	if (!(_cctx = avcodec_alloc_context3(_codec))) {
		Debug("Failed to allocate codec context", 0);
		Free();
		return;
	}

	_videoStream->codecpar->codec_id = _oformat->video_codec;
	_videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	_videoStream->codecpar->width = width;
	_videoStream->codecpar->height = height;
	_videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
	_videoStream->codecpar->bit_rate = bitrate * 1000;
	_videoStream->time_base = { 1, _fps };

	avcodec_parameters_to_context(_cctx, _videoStream->codecpar);
	_cctx->time_base = { 1, _fps };
	_cctx->max_b_frames = 2;
	_cctx->gop_size = 12;
	if (_videoStream->codecpar->codec_id == AV_CODEC_ID_H264) {
		av_opt_set(_cctx, "preset", "ultrafast", 0);
	}
	if (_ofctx->oformat->flags & AVFMT_GLOBALHEADER) {
		_cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	avcodec_parameters_from_context(_videoStream->codecpar, _cctx);

	if ((err = avcodec_open2(_cctx, _codec, NULL)) < 0) {
		Debug("Failed to open codec", err);
		Free();
		return;
	}

	if (!(_oformat->flags & AVFMT_NOFILE)) {
		if ((err = avio_open(&_ofctx->pb, _tempFilePath.c_str(), AVIO_FLAG_WRITE)) < 0) {
			Debug("Failed to open file", err);
			Free();
			return;
		}
	}

	if ((err = avformat_write_header(_ofctx, NULL)) < 0) {
		Debug("Failed to write header", err);
		Free();
		return;
	}

	av_dump_format(_ofctx, 0, _tempFilePath.c_str(), 1);
}

AVFrame* VideoCapture2::RotateRGBOrigData(uint8_t *data, int width, int height)
{
	AVFrame* inFrame = NULL;
	inFrame = av_frame_alloc();

    avpicture_fill((AVPicture*)inFrame, (uint8_t*)data, AV_PIX_FMT_RGB24, width, height);//第三个参数和输入图片的pixelFormat有关

	if (_needRotate)
	{
		inFrame->data[0] += inFrame->linesize[0] * (height - 1);
		inFrame->linesize[0] = -inFrame->linesize[0];
	}
	

	return inFrame;
}

//void VideoCapture2::AddFrame(beResource::Image* srcImg)
//{
//	beCore::Ptr<beResource::Image> img = srcImg;

//	if (img->getFormat() == beResource::IF_JPG)//JPG图片不需要翻转
//	{
//		img->convertFormat(beResource::IF_PNG);
//		_needRotate = false;
//	}

//	if (img->getPixelFormat() != beResource::PF_GL_BGRA)
//	{
//		img = srcImg->convertARGB();
//	}

//	uint8_t* data = (uint8_t*)img->getData();
//	int srcWidth = img->getWidth();
//	int srcHeight = img->getHeight();
//	AddFrame(data, srcWidth, srcHeight);

//}

void VideoCapture2::AddFrame(uint8_t *data, int srcWidth, int srcHeight) 
{
	//先对图像进行180°翻转，否则生成的视频是倒立的
	AVFrame* inFrame = RotateRGBOrigData(data, srcWidth, srcHeight);
	int err;
	if (!_videoFrame) 
	{
		_videoFrame = av_frame_alloc();
		_videoFrame->format = AV_PIX_FMT_YUV420P;
		_videoFrame->width = _cctx->width;
		_videoFrame->height = _cctx->height;

		if ((err = av_frame_get_buffer(_videoFrame, 32)) < 0) //AVFrame中为buffer分配内存
		{
			Debug("Failed to allocate picture", err);
			return;
		}
	}

	if (!_swsCtx) {
		//需要根据图片位深度设置第三个参数（这里处理的都是32位深度图片）
        _swsCtx = sws_getContext(srcWidth, srcHeight, AV_PIX_FMT_RGB24, _cctx->width, _cctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, 0, 0, 0);
    }

//	int inLinesize[1] = { 4 * _cctx->width };

    sws_scale(_swsCtx, (const uint8_t * const *)&(inFrame->data), inFrame->linesize, 0,
              srcHeight, _videoFrame->data, _videoFrame->linesize);

	_videoFrame->pts = _frameCounter++;

	if ((err = avcodec_send_frame(_cctx, _videoFrame)) < 0) 
	{
		Debug("Failed to send frame", err);
		return;
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	if (avcodec_receive_packet(_cctx, &pkt) == 0) 
	{
		pkt.flags |= AV_PKT_FLAG_KEY;
		av_interleaved_write_frame(_ofctx, &pkt);
		av_packet_unref(&pkt);
	}
	//释放临时对象
	av_frame_free(&inFrame);
}

void VideoCapture2::Finish() {
	//DELAYED FRAMES
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	for (;;) 
	{
		avcodec_send_frame(_cctx, NULL);
		if (avcodec_receive_packet(_cctx, &pkt) == 0) 
		{
			av_interleaved_write_frame(_ofctx, &pkt);
			av_packet_unref(&pkt);
		}
		else 
		{
			break;
		}
	}

	av_write_trailer(_ofctx);
	if (!(_oformat->flags & AVFMT_NOFILE)) 
	{
		int err = avio_close(_ofctx->pb);
		if (err < 0) {
			Debug("Failed to close file", err);
		}
	}

	Free();
	Remux();

	remove(_tempFilePath.c_str());//删除临时生成的h254文件
}

void VideoCapture2::Free() {
	if (_videoFrame) {
		av_frame_free(&_videoFrame);
        _videoFrame = nullptr;
	}
	if (_cctx) {
		avcodec_free_context(&_cctx);
        _cctx = nullptr;
	}
	if (_ofctx) {
		avformat_free_context(_ofctx);
        _ofctx = nullptr;
	}
	if (_swsCtx) {
		sws_freeContext(_swsCtx);
        _swsCtx = nullptr;
	}
}

void VideoCapture2::Remux() {
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	int err;

	if ((err = avformat_open_input(&ifmt_ctx, _tempFilePath.c_str(), 0, 0)) < 0) {
		Debug("Failed to open input file for remuxing", err);
		goto end;
	}
	if ((err = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		Debug("Failed to retrieve input stream information", err);
		goto end;
	}
	if ((err = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, _outFilePath.c_str()))) {
		Debug("Failed to allocate output context", err);
		goto end;
	}

	AVStream *inVideoStream = ifmt_ctx->streams[0];
	AVStream *outVideoStream = avformat_new_stream(ofmt_ctx, NULL);
	if (!outVideoStream) {
		Debug("Failed to allocate output video stream", 0);
		goto end;
	}
	outVideoStream->time_base = { 1, _fps };
	avcodec_parameters_copy(outVideoStream->codecpar, inVideoStream->codecpar);
	outVideoStream->codecpar->codec_tag = 0;

	if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		if ((err = avio_open(&ofmt_ctx->pb, _outFilePath.c_str(), AVIO_FLAG_WRITE)) < 0) {
			Debug("Failed to open output file", err);
			goto end;
		}
	}

	if ((err = avformat_write_header(ofmt_ctx, 0)) < 0) {
		Debug("Failed to write header to output file", err);
		goto end;
	}

	AVPacket videoPkt;
	int ts = 0;
	while (true) {
		if ((err = av_read_frame(ifmt_ctx, &videoPkt)) < 0) {
			break;
		}
		videoPkt.stream_index = outVideoStream->index;
		videoPkt.pts = ts;
		videoPkt.dts = ts;
		videoPkt.duration = av_rescale_q(videoPkt.duration, inVideoStream->time_base, outVideoStream->time_base);
		ts += videoPkt.duration;
		videoPkt.pos = -1;

		if ((err = av_interleaved_write_frame(ofmt_ctx, &videoPkt)) < 0) {
			Debug("Failed to mux packet", err);
			av_packet_unref(&videoPkt);
			break;
		}
		av_packet_unref(&videoPkt);
	}

	av_write_trailer(ofmt_ctx);

end:
	if (ifmt_ctx) {
		avformat_close_input(&ifmt_ctx);
	}
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
		avio_closep(&ofmt_ctx->pb);
	}
	if (ofmt_ctx) {
		avformat_free_context(ofmt_ctx);
	}
}
