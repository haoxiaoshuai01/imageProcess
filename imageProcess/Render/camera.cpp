#include "camera.h"
namespace  Engine{
Camera::Camera(const Eigen::Vector3f &_eye,const Eigen::Vector3f &_center,const Eigen::Vector3f &_up)
{
    lookatM.setIdentity();
    eye = _eye;
    center = _center;
    up = _up;
    updateLookAt();
}

void Camera::setEyeVecf(const Eigen::Vector3f &v)
{
    eye = v;
    updateLookAt();
}

void Camera::setCenterVecf(const Eigen::Vector3f &v)
{
    center = v;
    updateLookAt();
}

void Camera::setUpVecf(const Eigen::Vector3f &v)
{
    up = v;
    updateLookAt();
}

void Camera::updateLookAt()
{
    Eigen::Vector3f Direction_z = eye-center;
    Direction_z.normalize();

    Eigen::Vector3f Right_x =up.cross(Direction_z);
    Right_x.normalize();

    Eigen::Vector3f Up_y = Direction_z.cross(Right_x);
    Up_y.normalize();

    Eigen::Matrix4f minv;
    minv.setIdentity();


    minv.row(0) = Eigen::Vector4f(Right_x.x(),Right_x.y(),Right_x.z(),0.0f);
    minv.row(1) = Eigen::Vector4f(Up_y.x(),Up_y.y(),Up_y.z(),0.0f);
    minv.row(2) = Eigen::Vector4f(Direction_z.x(),Direction_z.y(),Direction_z.z(),0.0f);


    Eigen::Matrix4f tr;
    tr.setIdentity();
    tr.col(3) = Eigen::Vector4f(-eye.x(),-eye.y(),-eye.z(),1.0f);
    lookatM = minv*tr;

}
}
