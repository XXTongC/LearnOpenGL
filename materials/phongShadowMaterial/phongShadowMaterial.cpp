#include "phongShadowMaterial.h"
using namespace GLframework;

PhongShadowMaterial::PhongShadowMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::PhongShadowMaterial);
}

PhongShadowMaterial::~PhongShadowMaterial()
{

}


