#ifndef FACE_H
#define FACE_H
#include "Eigen/Dense"
#include <vector>
#include "Eigen/Core"
namespace  Engine{
class Mesh
{
public:
    enum TypeFace
    {
        Video = 0,
        Resource
    };

public:

    Mesh(const std::vector<Eigen::Vector3f> &_vertices, const std::vector<unsigned int> &_indices
         ,const std::vector<Eigen::Vector2f> &_TexCoords  = std::vector<Eigen::Vector2f>());
    virtual ~Mesh();
    virtual void draw();
    inline void setFaceType(TypeFace type){typeFace = type;};
    inline TypeFace getFaceType(){return typeFace ;}

    virtual void updateVerticesPos(const std::vector<Eigen::Vector3f> &_verticesPostion);
    void updateTexCoords(const std::vector<Eigen::Vector2f> &TexCoords);
    inline std::vector<Eigen::Vector3f> getVerticesPostion(){return verticesPostion;}
    inline std::vector<Eigen::Vector2f> getTexCoords(){return verticesTexCoords;}
    virtual void setAngles(const Eigen::Vector3f &angles);
    Eigen::Vector3f getAngles(){ return euler_angles; };

    virtual void setPosition(const Eigen::Vector3f &p);
    virtual void setScale(const Eigen::Vector3f &p);
    Eigen::Vector3f getPosition(){return position;};
    inline Eigen::Matrix4f getModelM(){return modelM;}
    inline void setDrawModel(unsigned int type){ drawModel = type;}
    inline bool getIsHideMesh(){return isHideMesh;};
    inline void setIsHideMesh(bool v){isHideMesh = v;};
    bool getFlipH(){return filpH;}
    bool getFlipV(){return filpV;}
    void setFilpH(bool v){filpH = v;}
    void setFilpV(bool v){filpV = v;}
private:
    void updateModelMatrix();
    // initializes all the buffer objects/arrays
    void setupMesh();
private:
    // mesh Data
    std::vector<Eigen::Vector3f> verticesPostion;
    std::vector<Eigen::Vector2f> verticesTexCoords;
    std::vector<unsigned int> indices;
    unsigned int VAO;
    unsigned int VBO[2];
    unsigned int EBO;

    Eigen::Vector3f euler_angles;
    Eigen::Matrix4f modelM;
    Eigen::Vector3f position;
    Eigen::Vector3f scale;
    unsigned int drawModel;
    bool isHideMesh = false;
    bool filpH = false;
    bool filpV = false;
    TypeFace typeFace;
};
}
#endif // FACE_H
