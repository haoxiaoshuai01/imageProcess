#ifndef GLOBAL_H
#define GLOBAL_H
#include "Eigen/Dense"
namespace  Engine{

Eigen::Matrix4f GenOrtho_M(float left, float right, float bottom,float top, float zNear, float zFar);
}


#endif // GLOBAL_H
