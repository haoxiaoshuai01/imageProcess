#include "RecordVideo.h"
#include <QObject>
#include "glad/glad.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>

Engine::RecordVideo::RecordVideo()
{

    recordVideo = new VideoCapture2();

    width = 1080;
    height = 1920;
    comp = 3;
}

Engine::RecordVideo::~RecordVideo()
{
    delete recordVideo;
}

void Engine::RecordVideo::draw(uchar *data)
{
    if(isStartRecord)
    {
      recordVideo->AddFrame(data,width,height);
    }
}

void Engine::RecordVideo::start(int delta)
{
    fps = 1000/delta;
    if(!isStartRecord)
    {
        recordVideo->Init(width, height, fps, 21000,
                    QString(QApplication::applicationDirPath()+"/"
                          +QDateTime::currentDateTime().
                          toString("yy_MM_dd_hh_mm_ss")
                            +".mp4").toStdString().data());
    }
    isStartRecord = true;
}

void Engine::RecordVideo::end()
{
    isStartRecord = false;
    recordVideo->Finish();
}
