#include "phongMaterial.h"

using namespace GLframework;

PhongMaterial::PhongMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png",1);
	setMaterialType(MaterialType::PhongMaterial);
}

PhongMaterial::~PhongMaterial()
{
	
}

float* PhongMaterial::shininessControl()
{
	return &mShiness;
}

const std::shared_ptr<Texture>& PhongMaterial::diffuseTexture() const
{
	return mDiffuse;
}

const std::shared_ptr<Texture>& PhongMaterial::specularMaskTexture() const
{
	return mSpecularMask;
}
