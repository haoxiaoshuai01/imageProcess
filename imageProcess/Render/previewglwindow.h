#ifndef GLWINDOW_H
#define GLWINDOW_H

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
#include "global.h"
#include <QTimer>
#include <map>
#include "../UI/source_info.h"
#include <QMenu>
#include "RecordVideo.h"
#include "../UI/win_dshow.h"
#include "PushStream.h"
#include "../UI/scene.h"
#include <unordered_map>
#include <QOpenGLWidget>
namespace  Engine{
class PreviewGlWindow : public QOpenGLWidget
{
    Q_OBJECT
public:
    struct SeletObjetInfo
    {
        FaceTexture *face  = nullptr;
        FaceTexture::SeleObject seleobjet = FaceTexture::SeleObject::non;
    };
    struct SourceDataTexture
    {
        int64_t intId;
        std::string uuid;
        Texture * texture;
        DataSource *data;
        bool isStart;
        FaceTexture *faceMesh;
        Scene_item *senceItem = nullptr;
    };
    struct VedioCameraData
    {
        Scene_item *item;
        Texture *textureVideo;
        FaceTexture *faceVideo;
    };
signals:
    void updateSenceTree();
    void stopPlayerRes(std::string uuid);
public:
    static PreviewGlWindow *Instance(){
        if(ptr == nullptr)
            return ptr = new PreviewGlWindow();
        else
            return ptr;
    };
    ~PreviewGlWindow();

    void addResource(AR_data_info *info);
    void beginPlayVedio(Scene_item * node);//{};
    void deleResource(AR_data_info *info);
    void deleResource(std::string id);
    void startResource(AR_data_info *info);
    void stopResource(AR_data_info *info);
    void hideResource(AR_data_info *info);
    void showResource(AR_data_info *info);
    void startRecord();
    void endRecord();
    void startStream();
    void endStream();
    void cancelSele();
    void deletVideo(std::string id);
    std::map<float,FaceTexture*> &getObject_z_sort(){return Object_z_sort;}
    std::map<std::string,SourceDataTexture> &getSourceDataMap(){return sourceDataMap;}

protected:
    void initializeGL() override;
    void resizeGL(int w ,int h) override;
    void paintGL() override;
protected:
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    void initMenu();
    void setSenceItemFilpV();
    void setSenceItemRotate();
    void setSenceItemFilpH();
    void leftCornerAlign(FaceTexture * face);
    void getCamerFrame();
    void addVedio(Scene_item *sceneItem,uint w,uint h,uint comp,unsigned char *data);

    Eigen::Vector3f mouseToWordPos(float _x,float _y);
    Eigen::Vector3f mouseDeltaToWord(float _x,float _y);
    Eigen::Vector3f ndcToWord(float ndcx,float ndcy,float ndcz);
    void hoverFace(const Eigen::Vector3f &nowMouse);
    void moveFace(const Eigen::Vector3f &nowMouse,const QPoint &deltaMouseWin);
    void seleFace(const Eigen::Vector3f &w);
    void setUPFinalFbo();
private:
     static PreviewGlWindow *ptr;
    explicit PreviewGlWindow();
    QMenu *menu = nullptr;

    long long lastFrameTime;
    long long frameDelta;
    Shader *shaderTexture;
    Shader *shaderColor;
    std::map<std::string,SourceDataTexture> sourceDataMap;
    //face sort z postion
    std::map<float,FaceTexture*> Object_z_sort;
    Camera *camera;
    Eigen::Matrix4f ortho_M;
    int window_w;
    int window_h;
    bool flagMousePress = false;
    QPoint mousePoint;
    bool menuShowActived = false;

    //std::unordered_map<int64_t,VedioCameraData> cameraTextureVidoMap;


    SeletObjetInfo currentSele;
    QTimer *timerVideo = nullptr;
    bool isUpdateTexTimerOutActived = false;
    Engine::RecordVideo *recordVideo;
    bool isRecordVideoTimerOutActived = false;
    Engine::PushStream *pushStream = nullptr;
    unsigned int FinalFboID;
    unsigned int FinalFboTexture;
    unsigned int pboPushSteam1[2];
    int m_FirstPboIndex = 0;
    Shader* screenShader;
    unsigned int screenFaceVAO, screenFaceVBO;
    bool isBeginPlayVedio = false;



};
}
#endif // GLWINDOW_H
