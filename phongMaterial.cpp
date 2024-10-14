#include "phongMaterial.h"
using namespace GLframework;

PhongMaterial::PhongMaterial()
{
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png",1);
	setMaterialType(MaterialType::PhongMaterial);
}

PhongMaterial::~PhongMaterial()
{
	
}


