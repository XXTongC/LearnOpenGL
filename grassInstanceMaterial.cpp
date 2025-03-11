#include "materials/grassInstanceMaterial/grassInstanceMaterial.h"

using namespace GLframework;

GrassInstanceMaterial::GrassInstanceMaterial()
{
	//setColorBlendState(true);
	//setDepthWrite(false);
	//setDepthTest(GL_FALSE);
	//setColorBlendState(true);
	

	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::GrassInstanceMaterial);
}

GrassInstanceMaterial::~GrassInstanceMaterial()
{

}

float GrassInstanceMaterial::getBrightness() const
{
	return mBrightness;
}

float GrassInstanceMaterial::getUVScale() const
{
	return mUVScale;
}

glm::vec3 GrassInstanceMaterial::getWindDirection() const
{
	return mWindDirection;
}

float GrassInstanceMaterial::getWindScale() const
{
	return mWindScale;
}

void GrassInstanceMaterial::setBrightness(float value)
{
	mBrightness = value;
}

void GrassInstanceMaterial::setUVScale(float value)
{
	mUVScale = value;
}

void GrassInstanceMaterial::setWindDirection(glm::vec3 value)
{
	mWindDirection = glm::normalize(value);
}

void GrassInstanceMaterial::setWindScale(float value)
{
	mWindScale = value;
}

float* GrassInstanceMaterial::Control_Brightness()
{
	return &mBrightness;
}

float* GrassInstanceMaterial::Control_UVScale()
{
	return &mUVScale;
}

glm::vec3* GrassInstanceMaterial::Control_WindDirection()
{
	return &mWindDirection;
}

float* GrassInstanceMaterial::Control_WindScale()
{
	return &mWindScale;
}

float* GrassInstanceMaterial::Control_PhaseScale()
{
	return &mPhaseScale;
}

float GrassInstanceMaterial::getPhaseScale() const
{
	return mPhaseScale;
}

void GrassInstanceMaterial::setPhaseScale(float value)
{
	mPhaseScale = value;
}

glm::vec3* GrassInstanceMaterial::Control_CloudBlackColor()
{
	return &mCloudBlackColor;
}

glm::vec3* GrassInstanceMaterial::Control_CloudWhiteColor()
{
	return &mCloudWhiteColor;
}

glm::vec3 GrassInstanceMaterial::getCloudBlackColor() const
{
	return mCloudBlackColor;
}

glm::vec3 GrassInstanceMaterial::getCloudWhiteColor() const
{
	return mCloudWhiteColor;
}

void GrassInstanceMaterial::seCloudWhiteColor(glm::vec3 value)
{
	mCloudWhiteColor = value;
}

void GrassInstanceMaterial::setCloudBlackColor(glm::vec3 value)
{
	mCloudBlackColor = value;
}

float* GrassInstanceMaterial::Control_CloudUVScale()
{
	return &mCloudUVScale;
}

void GrassInstanceMaterial::setCloudUVScale(float value)
{
	mCloudUVScale = value;
}

float GrassInstanceMaterial::getCloudUVScale() const
{
	return mCloudUVScale;
}

float* GrassInstanceMaterial::Control_CloudSpeed()
{
	return &mCloudSpeed;
}

void GrassInstanceMaterial::setCloudSpeed(float value)
{
	mCloudSpeed = value;
}

float GrassInstanceMaterial::getCloudSpeed() const
{
	return mCloudSpeed;
}

float* GrassInstanceMaterial::Control_CloudLerp()
{
	return &mCloudLerp;
}

void GrassInstanceMaterial::setCloudLerp(float value)
{
	mCloudLerp = value;
}

float GrassInstanceMaterial::getCloudLerp() const
{
	return mCloudLerp;
}

















