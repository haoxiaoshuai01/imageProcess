#include "facetexture.h"
#include <iostream>
#include <cmath>
#include "qdebug.h"
using namespace Eigen;
namespace  Engine{



// 计算 |p1 p2| X |p1 p|
float GetCross(const Eigen::Vector2f& p1, const Eigen::Vector2f& p2,const Eigen::Vector2f& p)
{
    return (p2.x() - p1.x()) * (p.y() - p1.y()) -(p.x() - p1.x()) * (p2.y() - p1.y());
}
//判断点是否在5X5的以原点为左下角的正方形内（便于测试）
/*
p1 p4
p2 p3
*/
bool IsPointInMatrix(const Eigen::Vector2f& p,const Eigen::Vector2f& p1,const Eigen::Vector2f& p2,const Eigen::Vector2f& p3,const Eigen::Vector2f& p4)
{
    return GetCross(p1,p2,p) * GetCross(p3,p4,p) >= 0 && GetCross(p2,p3,p) * GetCross(p4,p1,p) >= 0;
    //return false;
}

FaceTexture::FaceTexture(const std::vector<Eigen::Vector3f> &_vertices,
                         const std::vector<unsigned int> &_indices
                         ,const std::vector<Eigen::Vector2f> &_TexCoords ,
                         Shader * textureShader,
                         Shader * boxShader)
    :Mesh(_vertices,_indices,_TexCoords)
{
    vp.setIdentity();
    boxLine = nullptr;
    boxSeleCorner = nullptr;
    wordAlixsBox.resize(4);
    box.resize(4);
    word_cornerBox_top_left.resize(4);
    word_cornerBox_top_right.resize(4);
    word_cornerBox_bottom_left.resize(4);
    word_cornerBox_bottom_right.resize(4);
    cornerBox_top_left.resize(4);
    cornerBox_top_right.resize(4);
    cornerBox_bottom_left.resize(4);
    cornerBox_bottom_right.resize(4);

    cornerBox_top.resize(4);
    word_cornerBox_top.resize(4);
    cornerBox_left.resize(4);
    word_cornerBox_left.resize(4);
    cornerBox_bottom.resize(4);
    word_cornerBox_bottom.resize(4);
    cornerBox_right.resize(4);
    word_cornerBox_right.resize(4);

    shader = textureShader;
    shaderBoxLine = boxShader;

    imageRatio =(getVerticesPostion()[0].x()-getVerticesPostion()[3].x())/ (getVerticesPostion()[0].y()-getVerticesPostion()[1].y());
}

FaceTexture::~FaceTexture()
{
    if(boxLine)
    delete  boxLine;
    if(boxSeleCorner)
    delete  boxSeleCorner;
}

void FaceTexture::draw()
{
    if(Mesh::getIsHideMesh())
        return ;
    shader->use();
    shader->setInt("texture1",0);
    shader->setMat4("mvp",vp*getModelM());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    Mesh::draw();
    if(boxLine && isShowBox)
    {
        shaderBoxLine->use();
        shaderBoxLine->setMat4("mvp",vp*getModelM());
        shaderBoxLine->setVec3("color",colorBox);
        boxLine->draw();
        if(isShowCorner)
        boxSeleCorner->draw();
    }

}

void FaceTexture::initBox()
{

    updateBoxVPos();

    {
        std::vector<unsigned int> _indices;
        _indices.push_back(0);
        _indices.push_back(1);

        _indices.push_back(1);
        _indices.push_back(2);

        _indices.push_back(2);
        _indices.push_back(3);

        _indices.push_back(3);
        _indices.push_back(0);

        boxLine = new Mesh(box,_indices);
        boxLine->setDrawModel(GL_LINES);
    }
    initCornerBox();
    updateWordBox();
}

//void FaceTexture::shaderInit()
//{
//    if(shader == nullptr)
//    {

//    }
//}

/*
p1 p4
p2 p3
*/
FaceTexture::SeleObject FaceTexture::mouseSeleObjet(Eigen::Vector3f v)
{

    SeleObject ret = isMouseHoverCorner(v);
    if(ret)
        return ret;

     if(isMouseHover(v))
     {
        return SeleObject::Box;
     }
     return  non;
}

bool FaceTexture::isMouseHover(Eigen::Vector3f wordPos)
{
    return (IsPointInMatrix(Eigen::Vector2f(wordPos.x(),wordPos.y()),
                           Eigen::Vector2f(wordAlixsBox[3].x(),wordAlixsBox[3].y()),
            Eigen::Vector2f(wordAlixsBox[2].x(),wordAlixsBox[2].y()),
            Eigen::Vector2f(wordAlixsBox[1].x(),wordAlixsBox[1].y()),
            Eigen::Vector2f(wordAlixsBox[0].x(),wordAlixsBox[0].y())));

}

FaceTexture::SeleObject FaceTexture::isMouseHoverCorner(Vector3f v)
{
    if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                       Eigen::Vector2f(word_cornerBox_top_left[3].x(),word_cornerBox_top_left[3].y()),
                       Eigen::Vector2f(word_cornerBox_top_left[2].x(),word_cornerBox_top_left[2].y()),
                       Eigen::Vector2f(word_cornerBox_top_left[1].x(),word_cornerBox_top_left[1].y()),
                       Eigen::Vector2f(word_cornerBox_top_left[0].x(),word_cornerBox_top_left[0].y())))
    {
        return SeleObject::corner_t_l;
    }

     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_top_right[3].x(),word_cornerBox_top_right[3].y()),
                                Eigen::Vector2f(word_cornerBox_top_right[2].x(),word_cornerBox_top_right[2].y()),
                                Eigen::Vector2f(word_cornerBox_top_right[1].x(),word_cornerBox_top_right[1].y()),
                                Eigen::Vector2f(word_cornerBox_top_right[0].x(),word_cornerBox_top_right[0].y()))
             )
                 {
                     return SeleObject::corner_t_r;
                 }
    if( IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_bottom_left[3].x(),word_cornerBox_bottom_left[3].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_left[2].x(),word_cornerBox_bottom_left[2].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_left[1].x(),word_cornerBox_bottom_left[1].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_left[0].x(),word_cornerBox_bottom_left[0].y()))
            )
                {
                    return SeleObject::corner_b_l;
                }
     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_bottom_right[3].x(),word_cornerBox_bottom_right[3].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_right[2].x(),word_cornerBox_bottom_right[2].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_right[1].x(),word_cornerBox_bottom_right[1].y()),
                                Eigen::Vector2f(word_cornerBox_bottom_right[0].x(),word_cornerBox_bottom_right[0].y()))
             )
                 {
                     return SeleObject::corner_b_r;
                 }

     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_bottom[3].x(),word_cornerBox_bottom[3].y()),
                                Eigen::Vector2f(word_cornerBox_bottom[2].x(),word_cornerBox_bottom[2].y()),
                                Eigen::Vector2f(word_cornerBox_bottom[1].x(),word_cornerBox_bottom[1].y()),
                                Eigen::Vector2f(word_cornerBox_bottom[0].x(),word_cornerBox_bottom[0].y()))
             )
                 {
                     return SeleObject::corner_b;
                 }
     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_right[3].x(),word_cornerBox_right[3].y()),
                                Eigen::Vector2f(word_cornerBox_right[2].x(),word_cornerBox_right[2].y()),
                                Eigen::Vector2f(word_cornerBox_right[1].x(),word_cornerBox_right[1].y()),
                                Eigen::Vector2f(word_cornerBox_right[0].x(),word_cornerBox_right[0].y()))
             )
                 {
                     return SeleObject::corner_r;
                 }
     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_left[3].x(),word_cornerBox_left[3].y()),
                                Eigen::Vector2f(word_cornerBox_left[2].x(),word_cornerBox_left[2].y()),
                                Eigen::Vector2f(word_cornerBox_left[1].x(),word_cornerBox_left[1].y()),
                                Eigen::Vector2f(word_cornerBox_left[0].x(),word_cornerBox_left[0].y()))
             )
                 {
                     return SeleObject::corner_l;
                 }
     if(IsPointInMatrix(Eigen::Vector2f(v.x(),v.y()),
                                Eigen::Vector2f(word_cornerBox_top[3].x(),word_cornerBox_top[3].y()),
                                Eigen::Vector2f(word_cornerBox_top[2].x(),word_cornerBox_top[2].y()),
                                Eigen::Vector2f(word_cornerBox_top[1].x(),word_cornerBox_top[1].y()),
                                Eigen::Vector2f(word_cornerBox_top[0].x(),word_cornerBox_top[0].y()))
             )
                 {
                     return SeleObject::corner_t;
                 }


      return SeleObject::non;
}
void FaceTexture::updateWordBox()
{
    auto funtion = [this](const  std::vector<Eigen::Vector3f> &box1,std::vector<Eigen::Vector3f> &box2){
        for(size_t i=0;i<box1.size();i++)
        {
            Eigen::Vector4f v=getModelM() * Eigen::Vector4f(box1[i].x(),box1[i].y(),box1[i].z(),1.0f);
            box2[i] = v.hnormalized();
        }
    };
    funtion(box,wordAlixsBox);
    funtion(cornerBox_top_left,word_cornerBox_top_left);
    funtion(cornerBox_top_right,word_cornerBox_top_right);
    funtion(cornerBox_bottom_left,word_cornerBox_bottom_left);
    funtion(cornerBox_bottom_right,word_cornerBox_bottom_right);

    funtion(cornerBox_top,word_cornerBox_top);
    funtion(cornerBox_left,word_cornerBox_left);
    funtion(cornerBox_bottom,word_cornerBox_bottom);
    funtion(cornerBox_right,word_cornerBox_right);


}

void FaceTexture::updateBoxVPos()
{
    std::vector< Vector3f> vertix =   getVerticesPostion();
  box[0] = (Eigen::Vector3f(vertix[0].x()+1,
                 vertix[0].y()+1,
          vertix[0].z()));
  box[1] = (Eigen::Vector3f(vertix[1].x()+1,
                 vertix[1].y()-1,
          vertix[1].z()));
  box[2] = (Eigen::Vector3f(vertix[2].x()-1,
                 vertix[2].y()-1,
          vertix[2].z()));
  box[3] = (Eigen::Vector3f(vertix[3].x()-1,
                 vertix[3].y()+1,
          vertix[3].z()));

}

std::vector< Vector3f> FaceTexture::updateCornerBoxVpos()
{
    float corner_w = 5.0f;
    auto funtion1 = [corner_w](const Vector3f &center,std::vector<Eigen::Vector3f> &dest){
        dest[0]=(center+Eigen::Vector3f(corner_w,corner_w,0.01f));
        dest[1]=(Eigen::Vector3f(center.x()+corner_w,center.y()-corner_w,center.z()+0.01f));
        dest[2]=(Eigen::Vector3f(center.x()-corner_w,center.y()-corner_w,center.z()+0.01f));
        dest[3]=(Eigen::Vector3f(center.x()-corner_w,center.y()+corner_w,center.z()+0.01f));

    };
    funtion1(box[0],cornerBox_top_right);
    funtion1(box[1],cornerBox_bottom_right);
    funtion1(box[2],cornerBox_bottom_left);
    funtion1(box[3],cornerBox_top_left);

    funtion1((box[0]+box[3])/2,cornerBox_top);
    funtion1((box[0]+box[1])/2,cornerBox_right);
    funtion1((box[3]+box[2])/2,cornerBox_left);
    funtion1((box[1]+box[2])/2,cornerBox_bottom);

    std::vector< Vector3f> _vertices;
    auto funtion = [&_vertices](const std::vector<Eigen::Vector3f> &vs)
    {
        for(const auto &item:vs)
        {
            _vertices.push_back(item);
        }
    };
    funtion(cornerBox_top_right);
    funtion(cornerBox_bottom_right);
    funtion(cornerBox_bottom_left);
    funtion(cornerBox_top_left);

    funtion(cornerBox_top);
    funtion(cornerBox_right);
    funtion(cornerBox_left);
    funtion(cornerBox_bottom);

    return _vertices;
}
void FaceTexture::setAngles(const Eigen::Vector3f &angles)
{
    Mesh::setAngles(angles);
    updateWordBox();
}

void FaceTexture::setPosition(const Eigen::Vector3f &p)
{
    Mesh::setPosition(p);
    updateWordBox();
}

void FaceTexture::setScale(const Vector3f &p)
{
    Mesh::setScale(p);
    updateWordBox();
}

void FaceTexture::updateVerticesPos(const std::vector<Vector3f> &_verticesPostion)
{
    Mesh::updateVerticesPos(_verticesPostion);
    updateBoxVPos();
    std::vector< Vector3f> _vertices;
    _vertices = updateCornerBoxVpos();
    boxLine->updateVerticesPos(box);
    boxSeleCorner->updateVerticesPos(_vertices);
    updateWordBox();
}

void FaceTexture::update_drag_Corner(Eigen::Vector3f mouseWordPos,SeleObject object)
{

    Vector4f local4 = getModelM().inverse()*Vector4f(mouseWordPos.x(),mouseWordPos.y(),mouseWordPos.z(),1) ;
    Vector3f local = local4.hnormalized();
    std::vector<Vector3f> v =  getVerticesPostion();
    float h_old=  v[0].y()-v[2].y();
    float w_old = v[0].x()-v[2].x();
    float h_now=  0.0f;
    float w_now = 0.0f;
    switch (object)
    {
        case SeleObject::corner_b_l:
        {
            h_now=  -local.y()+v[0].y();
            w_now = -local.x()+v[0].x();
            if((h_now<=10) || (w_now<=10))
                return ;
            float hr = h_now/h_old;
            float wr = w_now/w_old;
            float h =0 ;
            float w = 0;
            if(hr > wr)
            {
                h = h_now;
                w = h_now*imageRatio;
            }else
            {
                w = w_now;
                h = w_now/imageRatio;
            }
            v[2] = Vector3f(v[0].x()-w,v[0].y()-h,v[2].z());
            v[1] = Vector3f(v[0].x(),v[0].y()-h,v[1].z());
            v[3] = Vector3f(v[0].x()-w,v[0].y(),v[3].z());
        };break;
        case SeleObject::corner_b_r:
        {
            h_now=  -local.y()+v[3].y();
            w_now = local.x()-v[3].x();
            if((h_now<=10) || (w_now<=10))
                return ;
            float hr = h_now/h_old;
            float wr = w_now/w_old;
            float h =0 ;
            float w = 0;
            if(hr > wr)
            {
                h = h_now;
                w = h_now*imageRatio;
            }else
            {
                w = w_now;
                h = w_now/imageRatio;
            }
            v[1] = Vector3f(v[3].x()+w,v[3].y()-h,v[1].z());
            v[2] = Vector3f(v[3].x(),v[3].y()-h,v[2].z());
            v[0] = Vector3f(v[3].x()+w,v[3].y(),v[0].z());
        };break;
        case SeleObject::corner_t_l:
        {
            h_now=  local.y()-v[1].y();
            w_now = v[1].x()-local.x();
            if((h_now<=10) || (w_now<=10))
                return ;
            float hr = h_now/h_old;
            float wr = w_now/w_old;
            float h =0 ;
            float w = 0;
            if(hr > wr)
            {
                h = h_now;
                w = h_now*imageRatio;
            }else
            {
                w = w_now;
                h = w_now/imageRatio;
            }
            v[3] = Vector3f(v[1].x()-w,v[1].y()+h,v[3].z());
            v[2] = Vector3f(v[1].x()-w,v[1].y(),v[2].z());
            v[0] = Vector3f(v[1].x(),v[1].y()+h,v[0].z());
        };break;
        case SeleObject::corner_t_r:
        {
            h_now=  local.y()-v[2].y();
            w_now = local.x()-v[2].x();
            if((h_now<=10) || (w_now<=10))
                return ;
            float hr = h_now/h_old;
            float wr = w_now/w_old;
            float h =0 ;
            float w = 0;
            if(hr > wr)
            {
                h = h_now;
                w = h_now*imageRatio;
            }else
            {
                w = w_now;
                h = w_now/imageRatio;
            }
            v[0] = Vector3f(v[2].x()+w,v[2].y()+h,v[0].z());
            v[1] = Vector3f(v[2].x()+w,v[2].y(),v[1].z());
            v[3] = Vector3f(v[2].x(),v[2].y()+h,v[3].z());
        };break;
        case SeleObject::corner_t:
        {
            h_now=  local.y()-v[2].y();
            if((h_now<=10) )
                return ;
            float h =0 ;
            float w = 0;
            h = h_now;
            w = h_now*imageRatio;
            Eigen::Vector3f center =(v[1]+v[2])/2;

            v[0] = Vector3f(center.x()+w/2,center.y()+h,v[0].z());
            v[1] = Vector3f(center.x()+w/2,center.y(),v[1].z());
            v[2] = Vector3f(center.x()-w/2,center.y(),v[2].z());
            v[3] = Vector3f(center.x()-w/2,center.y()+h,v[3].z());
        };break;
        case SeleObject::corner_b:
        {
            h_now=  -local.y()+v[0].y();
            if((h_now<=10) )
                return ;
            float h =0 ;
            float w = 0;
            h = h_now;
            w = h_now*imageRatio;
            Eigen::Vector3f center =(v[3]+v[0])/2;

            v[0] = Vector3f(center.x()+w/2,center.y(),v[0].z());
            v[1] = Vector3f(center.x()+w/2,center.y()-h,v[1].z());
            v[2] = Vector3f(center.x()-w/2,center.y()-h,v[2].z());
            v[3] = Vector3f(center.x()-w/2,center.y(),v[3].z());
        };break;
        case SeleObject::corner_l:
        {
            w_now=  -local.x()+v[0].x();
            if( (w_now<=10))
                return ;
            float h =0 ;
            float w = 0;
            w = w_now;
            h = w_now/imageRatio;
            Eigen::Vector3f center =(v[0]+v[1])/2;

            v[0] = Vector3f(center.x(),center.y()+h/2,v[0].z());
            v[1] = Vector3f(center.x(),center.y()-h/2,v[1].z());
            v[2] = Vector3f(center.x()-w,center.y()-h/2,v[2].z());
            v[3] = Vector3f(center.x()-w,center.y()+h/2,v[3].z());
        };break;
        case SeleObject::corner_r:
        {
            w_now=  local.x()-v[2].x();
            if( (w_now<=10))
                return ;
            float h =0 ;
            float w = 0;
            w = w_now;
            h = w_now/imageRatio;
            Eigen::Vector3f center =(v[2]+v[3])/2;

            v[0] = Vector3f(center.x()+w,center.y()+h/2,v[0].z());
            v[1] = Vector3f(center.x()+w,center.y()-h/2,v[1].z());
            v[2] = Vector3f(center.x(),center.y()-h/2,v[2].z());
            v[3] = Vector3f(center.x(),center.y()+h/2,v[3].z());
        };break;
        default:  return ;break;
        }
        FaceTexture::updateVerticesPos(v);
}

void FaceTexture::initCornerBox()
{

        std::vector< Vector3f> _vertices;
        _vertices = updateCornerBoxVpos();
        std::vector<unsigned int> _indices;
        for(unsigned int i =0 ;i<(_vertices.size()/4);i++)
        {
            _indices.push_back(0+(i)*4);
            _indices.push_back(1+(i)*4);
            _indices.push_back(3+(i)*4);
            _indices.push_back(1+(i)*4);
            _indices.push_back(2+(i)*4);
            _indices.push_back(3+(i)*4);
        }
        boxSeleCorner = new Mesh(_vertices,_indices);

}
}

