#ifndef ANIMATIONGLWINDOW_H
#define ANIMATIONGLWINDOW_H

#include <QObject>
#include "glad/glad.h"
#include <QOpenGLWindow>
#include <QTime>
#include "shader.h"
#include "mesh.h"
#include "texture.h"
#include "datasource.h"
#include "camera.h"
#include "Eigen/Dense"
#include "facetexture.h"
namespace  Engine{
class AnimationGlWindow : public QOpenGLWindow
{
    Q_OBJECT
public:
    enum PlayMode{
        Single = 0,
        Loop
    };
signals:
    void setPlayTime(unsigned int s);
public:
    AnimationGlWindow();
    ~AnimationGlWindow();
    void replaceResource(QString Dir,QString CoverName);
    void setZoom(int v);
    void setPlayMode(PlayMode v);
    void start();
    void stop();
protected:
    void initializeGL() override;
    void resizeGL(int w ,int h) override;
    void paintGL() override;
    void updateResource(int w,int h);

private:
    long long lastFrameTime;
    FaceTexture *faceVideo= nullptr;
    DataSource* dataSource = nullptr;
    Texture* textureVideo = nullptr;

    Shader *shaderTexture = nullptr;
    Shader *shaderColor = nullptr;
    Eigen::Matrix4f  ortho_M;
    PlayMode playMode = Single;
    QTimer *timer = nullptr;

    bool replaceActived = false;
    QString resDir;
    QString coverName;
};
}

#endif // ANIMATIONGLWINDOW_H
