#ifndef FACETEXTURE_H
#define FACETEXTURE_H
#include "mesh.h"
#include "shader.h"
#include "glad/glad.h"
#include "Eigen/Core"
#include "texture.h"
namespace  Engine{
class FaceTexture :public Mesh
{
public:

    enum SeleObject
    {
        non = 0,
        corner_t_l,
        corner_t_r,
        corner_b_l,
        corner_b_r,
        corner_t,
        corner_b,
        corner_l,
        corner_r,
        Box

    };
public:
    FaceTexture(const std::vector<Eigen::Vector3f> &_vertices, const std::vector<unsigned int> &_indices
                ,const std::vector<Eigen::Vector2f> &_TexCoords  = std::vector<Eigen::Vector2f>(),
             Shader * textureShader = nullptr,
             Shader * boxShader = nullptr
            );
    virtual ~FaceTexture();
    void draw() override;
    void setvp(const Eigen::Matrix4f &m){ vp = m;}
    void setTextureID(const unsigned int &v){ textureId = v;}
    void initBox();

    inline void setIsShowBox(bool f,Eigen::Vector3f color = Eigen::Vector3f(1,0,0)){isShowBox = f;colorBox =color; };
    inline void setIsShowCorner(bool v){isShowCorner = v;}
    //static void shaderInit();
    SeleObject mouseSeleObjet(Eigen::Vector3f wordPos);
    bool isMouseHover(Eigen::Vector3f wordPos);
    SeleObject isMouseHoverCorner(Eigen::Vector3f wordPos);
    void setAngles(const Eigen::Vector3f &angles) override;

    void setPosition(const Eigen::Vector3f &p) override;
    void setScale(const Eigen::Vector3f &p) override;
    void updateVerticesPos(const std::vector<Eigen::Vector3f> &_verticesPostion) override;
    void update_drag_Corner(Eigen::Vector3f mouseWordPos,SeleObject object);

    void setBoxColor(Eigen::Vector3f v){colorBox = v;}
    void setNameId(std::string _nameId){  nameId = _nameId;}
    void setIntId(int64_t id){  intId = id;}
    inline std::string getNameId(){ return nameId;}
    inline int64_t getIntId(){ return intId;}
private:
    void initCornerBox();
    void updateWordBox();
    void updateBoxVPos();
    std::vector< Eigen::Vector3f> updateCornerBoxVpos();
private:
    unsigned int textureId;
    Eigen::Matrix4f vp;
    //size 4
    std::vector<Eigen::Vector3f> box;
    std::vector<Eigen::Vector3f> wordAlixsBox;

    std::vector<Eigen::Vector3f> cornerBox_top_left;
    std::vector<Eigen::Vector3f> word_cornerBox_top_left;
    std::vector<Eigen::Vector3f> cornerBox_top_right;
    std::vector<Eigen::Vector3f> word_cornerBox_top_right;
    std::vector<Eigen::Vector3f> cornerBox_bottom_left;
    std::vector<Eigen::Vector3f> word_cornerBox_bottom_left;
    std::vector<Eigen::Vector3f> cornerBox_bottom_right;
    std::vector<Eigen::Vector3f> word_cornerBox_bottom_right;
    std::vector<Eigen::Vector3f> cornerBox_top;
    std::vector<Eigen::Vector3f> word_cornerBox_top;
    std::vector<Eigen::Vector3f> cornerBox_left;
    std::vector<Eigen::Vector3f> word_cornerBox_left;
    std::vector<Eigen::Vector3f> cornerBox_bottom;
    std::vector<Eigen::Vector3f> word_cornerBox_bottom;
    std::vector<Eigen::Vector3f> cornerBox_right;
    std::vector<Eigen::Vector3f> word_cornerBox_right;

    Mesh* boxLine;
    Mesh* boxSeleCorner;
    bool isShowBox = false;
    bool isShowCorner = true;
    Shader * shader;
    Shader * shaderBoxLine;

    float imageRatio = 1.0f;
    Eigen::Vector3f colorBox;

    std::string nameId;
    int64_t intId;
};
}

#endif // FACETEXTURE_H
