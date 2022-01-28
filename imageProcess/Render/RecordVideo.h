#ifndef RECORDVIDEO_H
#define RECORDVIDEO_H
#include "VideoCapture2.h"
#include <QTimer>
#include "../UI/win_dshow.h"
namespace  Engine{
    class RecordVideo
    {
    public:
        RecordVideo();
        ~RecordVideo();
        void draw(uchar *data);
        void start(int delta);
        void end();
        int getw(){return width;};
        int geth(){return height;};
        bool getIsStart(){return isStartRecord;}

    private:
        int width = 0 ;
        int height  = 0;
        int comp  = 0;
        bool isStartRecord = false;

        VideoCapture2 *recordVideo = nullptr;
        uint fps = 25;
    };
};
#endif // SHADER_H
