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


