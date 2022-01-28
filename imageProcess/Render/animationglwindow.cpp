#include "animationglwindow.h"
#include "time_m.h"
#include "qdebug.h"
#include "qapplication.h"
#include "texture.h"
#include "UI/window_basic_main.hpp"
using namespace Eigen;
namespace  Engine{
static const char *textureVS = "#version 430 core\n"
     "layout (location = 0) in vec3 aPos;\n"
     "layout (location = 1) in vec2 aTexCoord;\n"
     "out vec2 TexCoord;\n"
     "uniform mat4 mvp;\n"
     "void main()\n"
     "{\n"
     "   gl_Position = mvp*vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
     "   TexCoord = aTexCoord;\n"
     "}\0";
static const char *textureFS = "#version 430 core\n"
     "out vec4 FragColor;\n"
     "in vec2 TexCoord;\n"
     "uniform sampler2D texture1;\n"
     "void main()\n"
     "{\n"
     "   FragColor = texture(texture1, TexCoord);\n"
     "}\n\0";
static const char *colorVS = "#version 430 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "uniform mat4 mvp;\n"
      "void main()\n"
      "{\n"
      "   gl_Position = mvp*vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
      "}\0";
 static const char *colorFS = "#version 430 core\n"
      "out vec4 FragColor;\n"
      "uniform vec3 color;\n"
      "void main()\n"
      "{\n"
      "   FragColor = vec4(color,1.0f);\n"
      "}\n\0";
static int timeOutms = 33;
AnimationGlWindow::AnimationGlWindow()
{

    this->setSurfaceType(OpenGLSurface);
    QObject::connect(this,&QOpenGLWindow::frameSwapped,this,[this](){
        this->update();
    });
}

AnimationGlWindow::~AnimationGlWindow()
{
    if(timer)
    {
        timer->stop();
        delete  timer;
    }
    if(shaderTexture)
    delete shaderTexture;
    if(shaderColor)
    delete shaderColor;
    if(faceVideo)
        delete faceVideo;
    if(dataSource)
        delete  dataSource;
    if(textureVideo)
        delete  textureVideo;
}

void AnimationGlWindow::initializeGL()
{
    if(!gladLoadGL())
    {
        std::cout<<"init failed"<<std::endl;
        return;
    }
    lastFrameTime = systemtime();
    qDebug() << "GL_VENDOR:"
                  << (char *)glGetString(GL_VENDOR)
                  << "GL_RENDERER:"
                  << (char *)glGetString(GL_RENDERER)
                  << "GL_VERSION:"
                  << (char *)glGetString(GL_VERSION);

    shaderTexture = new Shader(textureVS,textureFS);
    shaderColor = new Shader(colorVS,colorFS);
    std::vector<Vector2f> _TexCoords;
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 0.0f));
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 0.0f));
    {
        std::vector<Vector3f> _vertices;
        _vertices.push_back(Eigen::Vector3f(1,  1,0));
        _vertices.push_back(Eigen::Vector3f(1, -1, 0));
        _vertices.push_back(Eigen::Vector3f(-1, -1, 0));
        _vertices.push_back(Eigen::Vector3f(-1,  1, 0));
        std::vector<unsigned int> _indices;
        _indices.push_back(0);
        _indices.push_back(1);
        _indices.push_back(3);
        _indices.push_back(1);
        _indices.push_back(2);
        _indices.push_back(3);
        faceVideo = new FaceTexture(_vertices,_indices,_TexCoords,shaderTexture,shaderColor);
        faceVideo->initBox();
        faceVideo->setIsShowBox(true);
        faceVideo->setIsShowCorner(false);


        timer = new QTimer();
        timer->setTimerType(Qt::PreciseTimer);
        connect(timer,&QTimer::timeout,[this](){
                makeCurrent();
                dataSource->updateFrameToNext();
                textureVideo->updateBuffer(dataSource->getCurrentData());
                if(playMode == Single)
                {
                    if(dataSource->isLastFrame())
                        timer->stop();
                }
        });


    }

}

void AnimationGlWindow::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    ortho_M =  GenOrtho_M(0.0f,w,0.0f,h,0.1f,100.0f);
    faceVideo->setvp(ortho_M);
    updateResource(w,h);

}

void AnimationGlWindow::paintGL()
{
    if(replaceActived)
    {
        replaceActived = false;

        if(dataSource)
            delete  dataSource;
        if(textureVideo)
            delete  textureVideo;
        dataSource = new DataSource(resDir,coverName);
        textureVideo = new Texture(dataSource->getCurrentData(),
                                   dataSource->getWidth(),
                                   dataSource->getHeight(),
                                   dataSource->getComp()
                                   );
        faceVideo->setTextureID(textureVideo->getTextureId());
        updateResource(this->width(),this->height());
        emit setPlayTime((unsigned int)((float)(timeOutms*dataSource->getCount())*0.001f));
    }


    glClearColor(49.0f/255.0f, 59.0f/255.0f, 81.0f/255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(textureVideo)
        faceVideo->draw();

//    long long frameDelta =systemtime()- lastFrameTime;
//    qDebug()<<"frame delta"<<frameDelta;
    //    lastFrameTime  =systemtime();
}

void AnimationGlWindow::updateResource(int w,int h)
{
    if(textureVideo == nullptr)
        return ;
    float ratio = textureVideo->get_w_h_ratio();
    float m_h = 0.0f;
    float m_w = 0.0f;
    float wd = 0.0f;
    float hd = 0.0f;
    if(w>h)
    {
        m_h = (float)h;
        m_w = m_h*ratio;
        wd = ((float)w-m_w)/2;
    }else
    {
        m_w = w;
        m_h = m_w/ratio;
        hd = ((float)h-m_h)/2;
    }
    //z轴反转搞的事
    float z = -50;
    std::vector<Vector3f> _vertices;
    _vertices.push_back(Eigen::Vector3f(m_w/2,m_h/2,z));
    _vertices.push_back(Eigen::Vector3f(m_w/2,-m_h/2, z));
    _vertices.push_back(Eigen::Vector3f(-m_w/2,-m_h/2, z));
    _vertices.push_back(Eigen::Vector3f(-m_w/2, m_h/2, z));
    faceVideo->updateVerticesPos(_vertices);
    faceVideo->setPosition({m_w/2+wd,m_h/2+hd,0});
}

void AnimationGlWindow::replaceResource(QString Dir,QString CoverName)
{
    replaceActived = true;
    resDir = Dir;
    coverName =CoverName;
}

void AnimationGlWindow::setZoom(int v)
{
    float s = (float )v/100.0f;
    if(faceVideo)
    faceVideo->setScale(Eigen::Vector3f(s,s,s));
}

void AnimationGlWindow::setPlayMode(AnimationGlWindow::PlayMode v)
{
    playMode = v;
}

void AnimationGlWindow::start()
{
    timer->start(timeOutms);
}

void AnimationGlWindow::stop()
{
    timer->stop();
}

}
