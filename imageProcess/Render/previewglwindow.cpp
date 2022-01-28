#include "previewglwindow.h"
#include "qdebug.h"

#include <QApplication>

#include "facetexture.h"
#include "QMouseEvent"
#include "time_m.h"
#include <map>
#include <QDateTime>
#include <QThread>
#include <QFileInfo>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "share/shareData.h"
using namespace Eigen;
namespace  Engine{
#define RECORD_DELTAL 40
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
PreviewGlWindow *PreviewGlWindow::ptr = nullptr;
std::mutex g_mutex_finalFboTexData;
std::condition_variable condition_PushStream;
std::condition_variable condition_Record;
static unsigned char *finalFboTexData = new unsigned char[1080*1920*3];
static Engine::PushStream *pushStreamPtr = nullptr;
static std::thread *threadPushStream = nullptr;
static Engine::RecordVideo *recordVideoPtr = nullptr;
static std::thread *threadRecord = nullptr;
PreviewGlWindow::PreviewGlWindow()
{
    ortho_M.setIdentity();
    QObject::connect(this,&QOpenGLWidget::frameSwapped,this,[this](){
        this->update();
    });
    initMenu();
    this->setMouseTracking(true);
    //

}

PreviewGlWindow::~PreviewGlWindow()
{
    timerVideo->stop();
    if(timerVideo)
        delete timerVideo;
    for(auto item:sourceDataMap)
    {
        if(item.second.data)
            delete  item.second.data;
        if(item.second.texture)
            delete  item.second.texture;
    }
    for(auto item:Object_z_sort)
    {
        if(item.second)
            delete item.second;
    }
    if(recordVideo)
        delete recordVideo;
    if(finalFboTexData)
        delete[] finalFboTexData;
    if(pushStream)
        delete pushStream;
}


static void recordThreadRun()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(g_mutex_finalFboTexData);
        condition_Record.wait(lck);
        //qDebug()<<"push one record";
        recordVideoPtr->draw(finalFboTexData);
    }
}
static void pushStreamThreadRun()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(g_mutex_finalFboTexData);
        condition_PushStream.wait(lck);
        //qDebug()<<"push one stream";
        pushStreamPtr->draw(finalFboTexData);
    }
}
void PreviewGlWindow::initializeGL()
{
    makeCurrent();
    if(!gladLoadGL())
    {
        std::cout<<"init failed"<<std::endl;
        return;
    }
    lastFrameTime = systemtime();
//    qDebug() << "GL_VENDOR:"
//                  << (char *)glGetString(GL_VENDOR)
//                  << "GL_RENDERER:"
//                  << (char *)glGetString(GL_RENDERER)
//                  << "GL_VERSION:"
//                  << (char *)glGetString(GL_VERSION);

    camera = new Camera(Eigen::Vector3f(0,0,3.0f),
                        Eigen::Vector3f(0,0.0f,-1.0f),
                        Eigen::Vector3f(0.0f,1.0f,0.0f)
                        );
    shaderTexture = new Shader(textureVS,textureFS);
    shaderColor = new Shader(colorVS,colorFS);

    timerVideo = new QTimer();
    timerVideo->setTimerType(Qt::PreciseTimer);
    connect(timerVideo,&QTimer::timeout,[this](){
        isUpdateTexTimerOutActived = true;
    });
    timerVideo->start(timeOutms);



    recordVideo = new Engine::RecordVideo();
    recordVideoPtr =recordVideo;
    pushStream = new Engine::PushStream();
    pushStreamPtr =pushStream;
    threadPushStream = new std::thread(pushStreamThreadRun);

    threadRecord =new std::thread(recordThreadRun);

    setUPFinalFbo();
}

void PreviewGlWindow::resizeGL(int w, int h)
{
    window_w = w;
    window_h = h;
    //glViewport(0, 0,w, h);
    ortho_M= GenOrtho_M(0.0f,1080.0f,0.0f,1920.0f,0.1f,100.0f);
    for(auto it:Object_z_sort)
    {
        it.second->setvp(ortho_M* camera->getMatrix());
    }
}
void PreviewGlWindow::getCamerFrame()
{
    if(isBeginPlayVedio)
    {
           Scene_t * sceneT =  get_current_scene();
           Scene_item *sceneItem =sceneT->first_item;
           Scene_item *node = sceneItem;
            while(node)
            {
                if(node->renderCameraData.isCaptureCameraTimerOutActived == false)
                {
                    node->renderCameraData.deltaTime_ms += frameDelta;
                    if(node->renderCameraData.deltaTime_ms>(long long)(node->out_frame->interval_ns/1000000))
                    {
                        node->renderCameraData.isCaptureCameraTimerOutActived = true;
                    }
                }
                if(node->renderCameraData.isCaptureCameraTimerOutActived)
                {
                   //qDebug()<<"tick_source try get data nodeid:"<<node->id;
                   if(tick_source(node,node->out_frame)==0)
                     {
                       node->renderCameraData.isCaptureCameraTimerOutActived = false;
                       node->renderCameraData.deltaTime_ms = 0;
                       auto inputCamera_frame = node->out_frame;
//                       qDebug()<<"tick_source ok:"<<inputCamera_frame->width<<
//                                 inputCamera_frame->height <<
//                                 inputCamera_frame->interval_ns
//                              <<node->v_flip<<node->rotate;

                       if((inputCamera_frame->width != node->renderCameraData.widthCompareUpdate)
                                 ||(inputCamera_frame->height != node->renderCameraData.heightCompareUpdate)
                                 )
                         {
                             addVedio(node,inputCamera_frame->width,inputCamera_frame->height,3,inputCamera_frame->data[0]);
                             node->renderCameraData.widthCompareUpdate =inputCamera_frame->width;
                             node->renderCameraData.heightCompareUpdate =inputCamera_frame->height;
                         }

                          std::string nodeId = QString::number(node->id).toStdString();
                         if(sourceDataMap[nodeId].texture)
                            sourceDataMap[nodeId].texture->updateBuffer(inputCamera_frame->data[0]);
                     }
                }
                node = node->next;
            }
    }
}

void PreviewGlWindow::paintGL()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FinalFboID);
    glViewport(0, 0,1080, 1920);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(isUpdateTexTimerOutActived)
    {
        isUpdateTexTimerOutActived = false;
        for(auto &item:sourceDataMap)
        {
            if(item.second.isStart)
            {
                item.second.data->updateFrameToNext();
                item.second.texture->updateBuffer( item.second.data->getCurrentData());
                if(item.second.data->isLastFrame())
                {
                    if(ShareData::Instance()->AR_data_infoMap.count(item.second.uuid))
                    {
                        if(ShareData::Instance()->AR_data_infoMap[item.second.uuid]->playMode == PlayMode::Single)
                        {
                                emit stopPlayerRes(item.second.uuid);
                        }
                    }
                }
            }
        }
    }
    getCamerFrame();
    for(auto it=Object_z_sort.begin();it!=Object_z_sort.end();it++)
    {
        //透明 混合 从远到近去画
        if(it->second)
        {
           it->second->draw();
        }
    }
    if(recordVideo->getIsStart() ||pushStream->getIsStart())
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[m_FirstPboIndex]);
        glReadPixels(0, 0, 1080, 1920,  GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[1 - m_FirstPboIndex]);
        void *data = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 1080*1920*3, GL_MAP_READ_BIT);
        m_FirstPboIndex = 1 - m_FirstPboIndex;
        //g_mutex_finalFboTexData.lock();
        memcpy(finalFboTexData,data,1080*1920*3);
        //g_mutex_finalFboTexData.unlock();
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
        if(recordVideo->getIsStart())
        {
           static long long deltaTime_ms = 0;
            if(isRecordVideoTimerOutActived == false)
            {
                deltaTime_ms += frameDelta;
                if(deltaTime_ms>(long long)(RECORD_DELTAL))
                {
                    isRecordVideoTimerOutActived = true;
                }
            }
            if(isRecordVideoTimerOutActived)
            {
                condition_Record.notify_one();
            }
        }
        if(pushStream->getIsStart())
        {
            condition_PushStream.notify_one();
        }
    }


    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0, 0,window_w, window_h);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    screenShader->use();
    glBindVertexArray(screenFaceVAO);
    glBindTexture(GL_TEXTURE_2D, FinalFboTexture);	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);

    frameDelta =systemtime()- lastFrameTime;
    lastFrameTime  =systemtime();

    //qDebug()<<"frame delta"<<frameDelta;

}

void PreviewGlWindow::addResource(AR_data_info *info)
{
    if(sourceDataMap.count(info->uuid))
        return ;

    auto getPosition = [this]()->float{
            float ret = 0.0f;
        if(Object_z_sort.size()>0)
        {
            auto map_at = Object_z_sort.end();
            map_at--;
            ret = map_at->second->getPosition().z() + 0.001f;
        }
            return ret;
    };
    makeCurrent();
    auto dataSource = new DataSource(info->root_dir.data(),QFileInfo(info->cover_frame).fileName());
    auto textureVideo = new Texture(dataSource->getCurrentData(),
                               dataSource->getWidth(),
                               dataSource->getHeight(),
                               dataSource->getComp()
                               );

    float m_w = textureVideo->getW();
    float m_h = textureVideo->getH();
    float hd = ((m_h/2.0f)>1920.0f)?(m_h/2.0f-1920.0f):(1920.0f-m_h/2.0f);
    std::vector<Vector2f> _TexCoords;
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 0.0f));
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 0.0f));


    {
        float z = 0;
        std::vector<Vector3f> _vertices;
        _vertices.push_back(Eigen::Vector3f(m_w/2,m_h/2,z));
        _vertices.push_back(Eigen::Vector3f(m_w/2,-m_h/2, z));
        _vertices.push_back(Eigen::Vector3f(-m_w/2,-m_h/2, z));
        _vertices.push_back(Eigen::Vector3f(-m_w/2, m_h/2, z));
        std::vector<unsigned int> _indices;
        _indices.push_back(0);
        _indices.push_back(1);
        _indices.push_back(3);
        _indices.push_back(1);
        _indices.push_back(2);
        _indices.push_back(3);
        auto faceVideo = new FaceTexture(_vertices,_indices,_TexCoords,shaderTexture,shaderColor);
        faceVideo->setFaceType(FaceTexture::TypeFace::Resource);
        faceVideo->setNameId(info->uuid);
        faceVideo->setPosition({m_w/2,hd,getPosition()});
        faceVideo->setvp(ortho_M* camera->getMatrix());
        faceVideo->initBox();

        faceVideo->setTextureID(textureVideo->getTextureId());
        sourceDataMap[info->uuid] = {0,info->uuid,textureVideo,dataSource,false,faceVideo,nullptr};
        Object_z_sort[faceVideo->getPosition().z()] = faceVideo;
        emit updateSenceTree();
    }
}

void PreviewGlWindow::beginPlayVedio(Scene_item * node)
{
    isBeginPlayVedio = true;
    addVedio(node,0,0,0,nullptr);

}
void PreviewGlWindow::deletVideo(std::string sceneItemId)
{
    if(sourceDataMap.count(sceneItemId) == 0)
        return;
    qDebug()<<"delet input :"<<sceneItemId.data();
    SourceDataTexture &t = sourceDataMap[sceneItemId];

    if(t.faceMesh == currentSele.face)
    {
        currentSele.face = nullptr;
        currentSele.seleobjet =  FaceTexture::SeleObject::non;
    }
    Object_z_sort.erase(t.faceMesh->getPosition().z());
    delete  t.data;
    delete  t.faceMesh;
    delete  t.texture;
    t.data = nullptr;
    t.faceMesh = nullptr;
    t.texture = nullptr;

    isBeginPlayVedio = false;
    remove_scene_item(t.senceItem->name);
    sourceDataMap.erase(sceneItemId);
    emit updateSenceTree();

}
void PreviewGlWindow::addVedio(Scene_item *sceneItem, uint w, uint h,uint comp,unsigned char *data)
{
    qDebug()<<"add vedio w h comp id"<<w<< h<< comp<<sceneItem->id;
    std::string sceneItemId = QString::number(sceneItem->id).toStdString();
    {
        if(sourceDataMap.count(sceneItemId))
        {
            qDebug()<<"delet input 1";
            SourceDataTexture &t = sourceDataMap[sceneItemId];
            if(t.faceMesh == currentSele.face)
            {
                currentSele.face = nullptr;
                currentSele.seleobjet =  FaceTexture::SeleObject::non;
            }
            delete  t.data;
            t.data = nullptr;
            delete  t.texture;
            t.texture = nullptr;
        }else
        {
            qDebug()<<"delet input 2";
        }

    }
    auto getPosition = [this]()->float{
        float ret = -0.001f;
        if(Object_z_sort.size()>0)
        {
            for(auto it = Object_z_sort.rbegin();it!=Object_z_sort.rend();it++)
              {
                    if(it->second->getFaceType() == Mesh::TypeFace::Video)
                    {
                        ret = it->second->getPosition().z() + 0.0001f;
                        break;
                    }
              }
        }
        return ret;
    };

    makeCurrent();

    auto textureVideo = new Texture(data,
                               w,
                               h,
                               comp
                               );


    float m_w = textureVideo->getW();
    float m_h = textureVideo->getH();
    float hd = ((m_h/2.0f)>1920.0f)?(m_h/2.0f-1920.0f):(1920.0f-m_h/2.0f);
    std::vector<Vector2f> _TexCoords;
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 0.0f));
    _TexCoords.push_back(Eigen::Vector2f(1.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 1.0f));
    _TexCoords.push_back(Eigen::Vector2f(0.0f, 0.0f));
    if(sourceDataMap.count(sceneItemId) == 0)
    {
        float z = 0;
        std::vector<Vector3f> _vertices;
        _vertices.push_back(Eigen::Vector3f(m_w/2,m_h/2,z));
        _vertices.push_back(Eigen::Vector3f(m_w/2,-m_h/2, z));
        _vertices.push_back(Eigen::Vector3f(-m_w/2,-m_h/2, z));
        _vertices.push_back(Eigen::Vector3f(-m_w/2, m_h/2, z));
        std::vector<unsigned int> _indices;
        _indices.push_back(0);
        _indices.push_back(1);
        _indices.push_back(3);
        _indices.push_back(1);
        _indices.push_back(2);
        _indices.push_back(3);
        auto faceVideo = new FaceTexture(_vertices,_indices,_TexCoords,shaderTexture,shaderColor);
        faceVideo->setFaceType(FaceTexture::TypeFace::Video);
        faceVideo->setNameId(sceneItem->name);
        faceVideo->setIntId(sceneItem->id);
        faceVideo->setPosition({m_w/2,hd,getPosition()});
        faceVideo->setvp(ortho_M* camera->getMatrix());
        faceVideo->initBox();
        faceVideo->setTextureID(textureVideo->getTextureId());
        sourceDataMap[sceneItemId] = {sceneItem->id,std::string(sceneItem->name),textureVideo,nullptr,false,faceVideo,sceneItem};
        Object_z_sort[getPosition()] = faceVideo;
    }else
    {
        auto face = sourceDataMap[sceneItemId].faceMesh;
        std::vector<Vector3f> _vertices;
        _vertices.push_back(Eigen::Vector3f(m_w/2,m_h/2,face->getPosition().z()));
        _vertices.push_back(Eigen::Vector3f(m_w/2,-m_h/2, face->getPosition().z()));
        _vertices.push_back(Eigen::Vector3f(-m_w/2,-m_h/2, face->getPosition().z()));
        _vertices.push_back(Eigen::Vector3f(-m_w/2, m_h/2, face->getPosition().z()));
        face->updateVerticesPos(_vertices);
        leftCornerAlign(face);
        sourceDataMap[sceneItemId].texture = textureVideo;
        face->setTextureID(textureVideo->getTextureId());
    }
    emit updateSenceTree();
}
void PreviewGlWindow::deleResource(std::string id)
{
    if(sourceDataMap.count(id) == 0)
        return ;
    SourceDataTexture t = sourceDataMap[id];

    if(t.faceMesh == currentSele.face)
    {
        currentSele.face = nullptr;
        currentSele.seleobjet =  FaceTexture::SeleObject::non;
    }
    Object_z_sort.erase(t.faceMesh->getPosition().z());
    delete  t.data;
    delete  t.faceMesh;
    delete  t.texture;
    sourceDataMap.erase(id);
    emit updateSenceTree();
}
void PreviewGlWindow::deleResource(AR_data_info *info)
{
    deleResource(info->uuid);
}

void PreviewGlWindow::startResource(AR_data_info *info)
{

    sourceDataMap[info->uuid].isStart = true;
}

void PreviewGlWindow::stopResource(AR_data_info *info)
{
    sourceDataMap[info->uuid].isStart = false;
}

void PreviewGlWindow::hideResource(AR_data_info *info)
{
    sourceDataMap[info->uuid].faceMesh->setIsHideMesh(true) ;
}

void PreviewGlWindow::showResource(AR_data_info *info)
{
    sourceDataMap[info->uuid].faceMesh->setIsHideMesh(false) ;
}

void PreviewGlWindow::startRecord()
{
    recordVideo->start(RECORD_DELTAL);
}

void PreviewGlWindow::endRecord()
{
    recordVideo->end();
}

void PreviewGlWindow::startStream()
{
    pushStream->start();
}

void PreviewGlWindow::endStream()
{
    pushStream->end();
}

void PreviewGlWindow::cancelSele()
{
    if(menuShowActived)
    {
        menuShowActived =false;
        return ;
    }
    currentSele.face = nullptr;
    currentSele.seleobjet = FaceTexture::SeleObject::non;
    for(auto it = Object_z_sort.rbegin();it!=Object_z_sort.rend();it++)
    {
        auto face = it->second;
        if(face->getIsHideMesh())
            continue;
        face->setIsShowBox(false);
    }
}


void PreviewGlWindow::mouseMoveEvent(QMouseEvent *ev)
{
    Eigen::Vector3f nowMouse = mouseToWordPos(ev->pos().x(),ev->pos().y());
    QPoint deltaMouseWin =  ev->pos() - mousePoint;

    if(flagMousePress)
    {
        moveFace(nowMouse,deltaMouseWin);
    }else
    {
        hoverFace(nowMouse);
    }
    mousePoint = ev->pos();
    return QOpenGLWidget::mouseMoveEvent(ev);
}

void PreviewGlWindow::mousePressEvent(QMouseEvent *ev)
{
     if(ev->button() & Qt::LeftButton)
     {
         flagMousePress = true;
         Eigen::Vector3f w =  mouseToWordPos(ev->pos().x(),ev->pos().y());
         seleFace(w);
     }
    if(ev->button() & Qt::RightButton)
    {
        if(currentSele.face)
        {
            menu->move(cursor().pos()); //让菜单显示的位置在鼠标的坐标上
            menu->show();
            menuShowActived = true;
        }
    }

    return QOpenGLWidget::mousePressEvent(ev);
}

void PreviewGlWindow::mouseReleaseEvent(QMouseEvent *ev)
{
    if(ev->button() & Qt::LeftButton)
    {
        flagMousePress = false;
    }
    return QOpenGLWidget::mouseReleaseEvent(ev);
}

void PreviewGlWindow::leaveEvent(QEvent *event)
{
    cancelSele();
    QOpenGLWidget::leaveEvent(event);
}
void PreviewGlWindow::leftCornerAlign(FaceTexture * face)
{
    uint leftTopIndex = 3;
    int id = (int)(face->getAngles().z()) / 90;
    if((id == 0)||(id == 4))
    {
        leftTopIndex = 3;
    }else
    {
        leftTopIndex = id-1;
    }

    Vector3f leftTop = face->getVerticesPostion()[leftTopIndex];
    Matrix4f m = face->getModelM();
    Vector3f v = m.block(0,0,3,3) * leftTop;
    Vector3f t = Vector3f(0,1920,face->getPosition().z()) -v;
    face->setPosition({t.x(),t.y(),face->getPosition().z()});
}
void PreviewGlWindow::setSenceItemRotate(){
    if(currentSele.face->getFaceType() == Mesh::TypeFace::Video)
    {
        auto name = QString::number(currentSele.face->getIntId()).toStdString();
        if(sourceDataMap.count(name))
        {
            if(sourceDataMap[name].senceItem)
            {
                sourceDataMap[name].senceItem->rotate =
                        360.0f-currentSele.face->getAngles().z();
            }
        }
    }
}
void PreviewGlWindow::setSenceItemFilpH(){
    if(currentSele.face->getFaceType() == Mesh::TypeFace::Video)
    {
        auto name = QString::number(currentSele.face->getIntId()).toStdString();
        if(sourceDataMap.count(name))
        {
            if(sourceDataMap[name].senceItem)
            {
                sourceDataMap[name].senceItem->h_flip  = currentSele.face->getFlipH();
            }
        }
    }
}
void PreviewGlWindow::setSenceItemFilpV(){
    if(currentSele.face->getFaceType() == Mesh::TypeFace::Video)
    {
        auto name = QString::number(currentSele.face->getIntId()).toStdString();
        if(sourceDataMap.count(name))
        {
            if(sourceDataMap[name].senceItem)
            {
                sourceDataMap[name].senceItem->v_flip  = currentSele.face->getFlipV();
            }
        }
    }
}
void PreviewGlWindow::initMenu()
{

    menu = new QMenu();
    QAction *actionR90 = new QAction(u8"rotate 90");
    QAction *actionR180 = new QAction(u8"rotate 180");
    QAction *actionFH = new QAction(u8"Flip Horizontal");
    QAction *actionFV = new QAction(u8"Flip vertical");
    menu->addAction(actionR90);
    menu->addAction(actionR180);
    menu->addAction(actionFH);
    menu->addAction(actionFV);
    connect(actionR90,&QAction::triggered,[this](){
        if(currentSele.face)
        {
            float z = currentSele.face->getAngles().z();
            currentSele.face->setAngles({0,0,float(((int)z+270)%360)});
            leftCornerAlign(currentSele.face);
            setSenceItemRotate();
        }
    });
    connect(actionR180,&QAction::triggered,[this](){
        if(currentSele.face)
        {
            float z = currentSele.face->getAngles().z();
            currentSele.face->setAngles({0,0,z-180});
            setSenceItemRotate();
        }
    });
    connect(actionFH,&QAction::triggered,[this](){

        if(currentSele.face)
        {
           std::vector<Vector2f> _TexCoords =  currentSele.face->getTexCoords();
            Eigen::Vector2f tmp;

            tmp =_TexCoords[3];
           _TexCoords[3] =_TexCoords[0] ;
            _TexCoords[0] = tmp;

            tmp =_TexCoords[2];
           _TexCoords[2] =_TexCoords[1] ;
            _TexCoords[1] = tmp;

            currentSele.face->updateTexCoords(_TexCoords );
            currentSele.face->setFilpH(!currentSele.face->getFlipH());
            setSenceItemFilpH();

        }
    });
    connect(actionFV,&QAction::triggered,[this](){
        if(currentSele.face)
        {
           std::vector<Vector2f> _TexCoords =  currentSele.face->getTexCoords();
            Eigen::Vector2f tmp;

            tmp =_TexCoords[3];
           _TexCoords[3] =_TexCoords[2] ;
            _TexCoords[2] = tmp;

            tmp =_TexCoords[0];
           _TexCoords[0] =_TexCoords[1] ;
            _TexCoords[1] = tmp;

            currentSele.face->updateTexCoords(_TexCoords );
            currentSele.face->setFilpV(!currentSele.face->getFlipV());
            setSenceItemFilpV();

        }
    });
}



Eigen::Vector3f PreviewGlWindow::mouseToWordPos(float _x, float _y)
{
    float x = _x;
    float y = (window_h-_y);

    float ndcx = (x-window_w/2)/(window_w/2);
    float ndcy = (y-window_h/2)/(window_h/2);
    float ndcz = 1.0f;
    return ndcToWord(ndcx,ndcy,ndcz);

}

Vector3f PreviewGlWindow::mouseDeltaToWord(float _x, float _y)
{
    float x = _x;
    float y = -_y;
    float ndcx = x/(window_w/2);
    float ndcy = y/(window_h/2);
    Eigen::Vector4f ndc(ndcx,ndcy,0.0f,0.0f);
    Eigen::Vector4f wordPos =  camera->getMatrix().inverse()*ortho_M.inverse()*ndc;
    return Vector3f(wordPos.x(),wordPos.y(),0.0f);
}

Eigen::Vector3f PreviewGlWindow::ndcToWord(float ndcx, float ndcy, float ndcz)
{
    Eigen::Vector4f ndc(ndcx,ndcy,ndcz,1.0f);
    Eigen::Vector4f wordPos =  camera->getMatrix().inverse()*ortho_M.inverse()*ndc;
    return wordPos.hnormalized();
}

void PreviewGlWindow::hoverFace(const Eigen::Vector3f &nowMouse)
{
    if(currentSele.face)
    {
        FaceTexture::SeleObject ret =currentSele.face->isMouseHoverCorner(nowMouse);
        int angleType = ((int)(currentSele.face->getAngles().z())/90)%4;
        switch (ret) {
        case FaceTexture::corner_t_r:
        case FaceTexture::corner_b_l:{

            if((angleType==0)||(angleType==2))
            {
              setCursor(Qt::SizeBDiagCursor);
            }else
            {
               setCursor(Qt::SizeFDiagCursor);
            }

            };break;
        case FaceTexture::corner_b_r:
        case FaceTexture::corner_t_l:{
            if((angleType==0)||(angleType==2))
            {
               setCursor(Qt::SizeFDiagCursor);
            }else
            {
               setCursor(Qt::SizeBDiagCursor);
            }
        };
            break;
        case FaceTexture::corner_t:
        case FaceTexture::corner_b:
            {
            if((angleType==0)||(angleType==2))
            {
               setCursor(Qt::SizeVerCursor);
            }else
            {
               setCursor(Qt::SizeHorCursor);
            }

            };
            break;
        case FaceTexture::corner_r:
        case FaceTexture::corner_l:
            {
            if((angleType==0)||(angleType==2))
            {
                setCursor(Qt::SizeHorCursor);

            }else
            {
               setCursor(Qt::SizeVerCursor);
            }

            };
            break;
        default:setCursor(Qt::ArrowCursor);break;
        }
    }
    bool flagFirst = true;
    for(auto it = Object_z_sort.rbegin();it!=Object_z_sort.rend();it++)
    {
        auto face = it->second;
        if(face->getIsHideMesh())
            continue;
        //if(face->getFaceType() == FaceTexture::Resource)
        {

            if(flagFirst)
            {
                if(face->isMouseHover(nowMouse))
                {
                    flagFirst = false;
                    if(face != currentSele.face)
                        face->setIsShowBox(true,Vector3f(0,0,1));
                }else
                {
                    if(face != currentSele.face)
                        face->setIsShowBox(false);
                }
            }else
            {
                if(face != currentSele.face)
                    face->setIsShowBox(false);
            }

        }
    }

}

void PreviewGlWindow::moveFace(const Vector3f &nowMouse,const QPoint &deltaMouseWin)
{
    if(currentSele.face)
    {
        if(currentSele.seleobjet == FaceTexture::Box)
        {
           Eigen::Vector3f delta =   mouseDeltaToWord(deltaMouseWin.x(),deltaMouseWin.y());
           currentSele.face->setPosition(currentSele.face->getPosition()+delta);

        } else
        {
            currentSele.face->update_drag_Corner(nowMouse,currentSele.seleobjet);
        }
    }
}

void PreviewGlWindow::seleFace(const Eigen::Vector3f &w)
{

    bool flagFirst = true;
    for(auto it = Object_z_sort.rbegin();it!=Object_z_sort.rend();it++)
    {
        auto face = it->second;
        if(face->getIsHideMesh())
            continue;
        //if(face->getFaceType() == FaceTexture::Resource)
        {
            if(flagFirst)
            {
                FaceTexture::SeleObject s =  face->mouseSeleObjet(w);
                if(s)
                {
                    flagFirst = false;
                    currentSele.face= face;
                    currentSele.seleobjet = s;
                    face->setIsShowBox(true);
                }else
                {
                   face->setIsShowBox(false);
                }
            }else
            {
                face->setIsShowBox(false);
            }
        }
    }
    if(flagFirst)
    {
        currentSele.face = nullptr;
        currentSele.seleobjet = FaceTexture::SeleObject::non;
    }

}

void PreviewGlWindow::setUPFinalFbo()
{
    // framebuffer configuration
        // -------------------------
        int w = 1080;
        int h = 1920;

        glGenFramebuffers(1, &FinalFboID);
        glBindFramebuffer(GL_FRAMEBUFFER, FinalFboID);
        // create a color attachment texture

        glGenTextures(1, &FinalFboTexture);
        glBindTexture(GL_TEXTURE_2D, FinalFboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FinalFboTexture, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        int w2 = w;
        int h2 = h;
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,w2, h2); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());

        float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
               // positions   // texCoords
               -1.0f,  1.0f,  0.0f, 1.0f,
               -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

               -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
           };
        // screen quad VAO

        glGenVertexArrays(1, &screenFaceVAO);
        glGenBuffers(1, &screenFaceVBO);
        glBindVertexArray(screenFaceVAO);
        glBindBuffer(GL_ARRAY_BUFFER, screenFaceVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        screenShader = new Shader(
                    "#version 330 core                                   \n"
                    "layout (location = 0) in vec2 aPos;                 \n"
                    "layout (location = 1) in vec2 aTexCoords;           \n"
                    "                                                    \n"
                    "out vec2 TexCoords;                                 \n"
                    "                                                    \n"
                    "void main()                                         \n"
                    "{                                                   \n"
                    "    TexCoords = aTexCoords;                         \n"
                    "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);   \n"
                    "}                                                   \n"
                    ,
                    "#version 330 core                                    \n"
                    "out vec4 FragColor;                                  \n"
                    "                                                     \n"
                    "in vec2 TexCoords;                                   \n"
                    "                                                     \n"
                    "uniform sampler2D screenTexture;                     \n"
                    "                                                     \n"
                    "void main()                                          \n"
                    "{                                                    \n"
                    "    vec3 col = texture(screenTexture, TexCoords).rgb;\n"
                    "    FragColor = vec4(col, 1.0);                      \n"
                    "}                                                    \n"
                    );
        screenShader->use();
        screenShader->setInt("screenTexture", 0);

        glGenBuffers(2,pboPushSteam1);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[0]);
        glBufferData(GL_PIXEL_PACK_BUFFER,1920*1080*3, nullptr, GL_STREAM_READ);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboPushSteam1[1]);
        glBufferData(GL_PIXEL_PACK_BUFFER,1920*1080*3, nullptr, GL_STREAM_READ);
}

}
