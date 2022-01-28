#include "global.h"

namespace  Engine{
Eigen::Matrix4f GenOrtho_M(float left, float right, float bottom,float top, float zNear, float zFar)
{
    Eigen::Matrix4f ortho_M;
    ortho_M.setIdentity();
    ortho_M(0,0) = 2/(right-left);
    ortho_M(1,1) = 2/(top-bottom);
    ortho_M(2,2) = 2/(zNear-zFar);
    //暗含z轴反转 //opengl 默认越小通过深度测试
    ortho_M.col(3) = Eigen::Vector4f(-(right+left)/(right-left),
                                     -(bottom+top)/( top - bottom),
                                     (zNear+zFar)/(zNear-zFar),1.0f);
    return ortho_M;
}
}
