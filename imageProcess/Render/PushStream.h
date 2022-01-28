#ifndef PUSHSTREAM_H
#define PUSHSTREAM_H
#include <QTimer>
#include "../UI/win_dshow.h"

namespace  Engine{
    class PushStream
    {
    public:
        PushStream();
        ~PushStream();
        void draw(uchar *data);
        void start();
        void end();
        int getw(){return width;};
        int geth(){return height;};
        bool getIsStart(){return isStartPushStream;}
    private:
        int width = 1080;
        int height  = 1920;
        int comp  = 3;
        tick_frame *in_frame;
        bool isStartPushStream = false;
    };
};
#endif // SHADER_H
