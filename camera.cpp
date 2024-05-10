#include "camera.h"


Camera::~Camera()
{
	
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(mPosition, front, center);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::identity<glm::mat4>();
}

