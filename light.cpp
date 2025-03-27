#include "light.h"

using namespace GLframework;



std::shared_ptr<Shadow> Light::getShadow() const
{
	return mShadow;
}

void Light::setShadow(std::shared_ptr<Shadow> value)
{
	mShadow = std::move(value);
}


glm::vec3 Light::getColor() const
{
	return mColor;
}

float Light::getSpecularIntensity() const
{
	return mSpecularIntensity;
}

void Light::setColor(glm::vec3 value)
{
	mColor = value;
}

void Light::setSpecularIntensity(float value)
{
	mSpecularIntensity = value;
}

float Light::getIntensity() const
{
	return mIntensity;
}

void Light::setIntensity(float value)
{
	mIntensity = value;
}


float* Light::Control_Intensity()
{
	return &mIntensity;
}
