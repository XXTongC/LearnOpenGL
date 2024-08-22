#include "camera.h"

Camera::~Camera()
{
	
}

void Camera::scale(float deltaScale)
{
	
}

glm::mat4 Camera::getViewMatrix() const
{
	glm::vec3 front = glm::cross(mUp, mRight);
	glm::vec3 center = mPosition + front;
	return glm::lookAt(mPosition, center, mUp);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::identity<glm::mat4>();
}

