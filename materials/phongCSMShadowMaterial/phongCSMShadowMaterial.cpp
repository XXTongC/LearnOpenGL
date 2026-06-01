#include "phongCSMShadowMaterial.h"

using namespace GLframework;

float* PhongCSMShadowMaterial::shininessControl()
{
	return &mShiness;
}

const std::shared_ptr<Texture>& PhongCSMShadowMaterial::diffuseTexture() const
{
	return mDiffuse;
}

const std::shared_ptr<Texture>& PhongCSMShadowMaterial::specularMaskTexture() const
{
	return mSpecularMask;
}
