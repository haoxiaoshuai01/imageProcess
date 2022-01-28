#include "glad/glad.h"
#include "mesh.h"
#include "QDebug"
#include <iostream>
#include "time_m.h"

#define pi 3.1415926f
namespace  Engine{
Mesh::Mesh(const std::vector<Eigen::Vector3f> &_vertices,
           const std::vector<unsigned int> &_indices,
           const std::vector<Eigen::Vector2f> &_TexCoords)
{
    typeFace = Resource;
    verticesPostion = _vertices;
    verticesTexCoords = _TexCoords;
    indices = _indices;
    modelM.setIdentity();
    euler_angles.setZero();
    position.setZero();
    scale.setOnes();
    drawModel =GL_TRIANGLES;
    setupMesh();
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::draw()
{
    if(isHideMesh)
        return ;
    glBindVertexArray(VAO);
    glDrawElements(drawModel, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, verticesPostion.size() * sizeof(Eigen::Vector3f), &verticesPostion[0], GL_DYNAMIC_DRAW);
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector3f), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    if(verticesTexCoords.size())
    {
        glBufferData(GL_ARRAY_BUFFER, verticesTexCoords.size() * sizeof(Eigen::Vector2f), &verticesTexCoords[0], GL_DYNAMIC_DRAW);
    }else
    {
        glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_STATIC_DRAW);
    }

    //vertex texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector2f), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void Mesh::updateVerticesPos(const std::vector<Eigen::Vector3f> &_verticesPostion)
{
    verticesPostion = _verticesPostion;
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER,0,_verticesPostion.size() * sizeof(Eigen::Vector3f),&_verticesPostion[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Mesh::updateTexCoords(const std::vector<Eigen::Vector2f> &TexCoords)
{
    verticesTexCoords = TexCoords;
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferSubData(GL_ARRAY_BUFFER,0,verticesTexCoords.size() * sizeof(Eigen::Vector2f),&verticesTexCoords[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::setAngles(const Eigen::Vector3f &angles)
{
    euler_angles = angles;
    updateModelMatrix();
}

void Mesh::setPosition(const Eigen::Vector3f &p)
{
    position = p;
    updateModelMatrix();
}

void Mesh::setScale(const Eigen::Vector3f &p)
{
    scale = p;
    updateModelMatrix();
}
/*Z1Y2X3 =
c1c2 c1s2s3-c3s1 s1s3+c1c3s2
c2s1 c1c3+s1s2s3 c3s1s2-c1s3
-s2  c2s3        c2c3
*/
void Mesh::updateModelMatrix()
{
    modelM.setIdentity();
    float c1 = cosf(euler_angles.z() *  pi/180);
    float s1 = sinf(euler_angles.z() *  pi/180);

    float c2 = cosf(euler_angles.y() *  pi/180);
    float s2 = sinf(euler_angles.y() *  pi/180);

    float c3 = cosf(euler_angles.x() *  pi/180);
    float s3 = sinf(euler_angles.x() *  pi/180);


    modelM.block(0,0,3,3)
            <<(c1*c2)*scale.x(),c1*s2*s3-c3*s1, s1*s3+c1*c3*s2,
              c2*s1,(c1*c3+s1*s2*s3)*scale.y(), c3*s1*s2-c1*s3,
              -s2,c2*s3,(c2*c3)*scale.z();

    modelM.col(3) =Eigen::Vector4f(position.x(),position.y(),position.z(),1.0f);

}
}
