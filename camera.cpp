#include "camera.h"

Camera::~Camera()
{
	
}

void Camera::scale(float deltaScale)
{
	
}

glm::mat4 Camera::getViewMatrix() const
{
	//lookat 
	//-eye:相机位置（使用mPosition）
	//-center：看向世界坐标的哪个点
	//-top：穹顶（使用mUp替代）
	glm::vec3 front = glm::cross(mUp, mRight);
	glm::vec3 center = mPosition + front;
	return glm::lookAt(mPosition, center, mUp);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::identity<glm::mat4>();
}

