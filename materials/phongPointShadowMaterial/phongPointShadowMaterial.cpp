#include "phongPointShadowMaterial.h"

using namespace GLframework;

PhongPointShadowMaterial::PhongPointShadowMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::PhongPointShadowMaterial);
}

PhongPointShadowMaterial::~PhongPointShadowMaterial()
{

}

float* PhongPointShadowMaterial::shininessControl()
{
	return &mShiness;
}

const std::shared_ptr<Texture>& PhongPointShadowMaterial::diffuseTexture() const
{
	return mDiffuse;
}

const std::shared_ptr<Texture>& PhongPointShadowMaterial::specularMaskTexture() const
{
	return mSpecularMask;
}
