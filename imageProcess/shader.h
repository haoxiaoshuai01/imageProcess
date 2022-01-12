#ifndef SHADER_H
#define SHADER_H
#include "Eigen/Dense"
#include "iostream"
#include "string"
#include "glad/glad.h"

class Shader
{
public:
    Shader(const char* vShaderCode, const char* fShaderCode);
    ~Shader();
    void use() const;
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const Eigen::Vector2f &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const Eigen::Vector3f &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const Eigen::Vector4f &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;
    void setMat2(const std::string &name, const Eigen::Matrix2f &mat) const;
    void setMat3(const std::string &name, const Eigen::Matrix3f &mat) const;
    void setMat4(const std::string &name, const Eigen::Matrix4f &mat) const;
private:
   unsigned int ID;
   void checkCompileErrors(GLuint shader, std::string type);

};


#endif // SHADER_H
