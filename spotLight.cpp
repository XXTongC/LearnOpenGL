#include "spotLight.h"
using namespace GLframework;

void SpotLight::setTargetDirection(glm::vec3 value)
{
	mTargetDirection = value;
}
/*
void SpotLight::setVisibleAngle(float value)
{
	mVisibleAngle = value;
}
*/

float SpotLight::getInnerAngle() const
{
	return mInnerAngle;
}

float SpotLight::getOutAngle() const
{
	return mOutAngle;
}

void SpotLight::setInnerAngle(float value)
{
	mInnerAngle = value;
}

void SpotLight::setOutAngle(float value)
{
	mOutAngle = value;
}





glm::vec3 SpotLight::getTargetDirection() const
{
	return mTargetDirection;
}

/*
float SpotLight::getVisibleAngle() const
{
	return mVisibleAngle;
}
*/
