#include "phongInstanceMaterial.h"
using namespace GLframework;

PhongInstanceMaterial::PhongInstanceMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::PhongInstanceMaterial);
}

PhongInstanceMaterial::~PhongInstanceMaterial()
{

}


