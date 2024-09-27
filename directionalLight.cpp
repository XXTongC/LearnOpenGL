#include "directionalLight.h"

using namespace GLframework;

glm::vec3 DirectionalLight::getDirection() const
{
	return mDirection;
}
void DirectionalLight::setDirection(glm::vec3 value)
{
	mDirection = value;
}
