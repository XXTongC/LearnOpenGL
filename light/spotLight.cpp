#include "spotLight.h"
using namespace GLframework;




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


