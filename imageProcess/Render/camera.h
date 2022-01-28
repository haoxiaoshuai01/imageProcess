#ifndef CAMERA_H
#define CAMERA_H

#include "Eigen/Core"
#include "Eigen/Dense"
namespace  Engine{
    class Camera
    {
    public:
        Camera(const Eigen::Vector3f &_eye,const Eigen::Vector3f &_center,const Eigen::Vector3f &_up);
        inline Eigen::Matrix4f getMatrix(){
            //return Eigen::Matrix4f::Identity();
            return lookatM;
        }
        void setEyeVecf(const Eigen::Vector3f &v);
        void setCenterVecf(const Eigen::Vector3f &v);
        Eigen::Vector3f getCenter(){return center;}
        void setUpVecf(const Eigen::Vector3f &v);
    private:
        void updateLookAt();
        Eigen::Vector3f eye;
        Eigen::Vector3f center;
        Eigen::Vector3f up;
        Eigen::Matrix4f lookatM;
    };
}

#endif // CAMERA_H
