#include "PushStream.h"
#include "qdebug.h"


Engine::PushStream::PushStream()
{
    in_frame = new tick_frame;
}

Engine::PushStream::~PushStream()
{
    delete in_frame;
}

void Engine::PushStream::draw(uchar *data)
{
    in_frame->data[0] = data;
    in_frame->width = getw();
    in_frame->height = geth();
    //qDebug()<<(in_frame->data[0][0])<<in_frame->data[0][1]<<in_frame->data[0][2];
    send_vcam_data(g_vcam, in_frame,g_vcam_enabled);
    //qDebug()<<"send_vcam_data";
}

void Engine::PushStream::start()
{
   isStartPushStream = true;
}

void Engine::PushStream::end()
{
    isStartPushStream = false;
}
