#include "phongNormalMaterial.h"
using namespace GLframework;

PhongNormalMaterial::PhongNormalMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/FFFFFF.png",1);
	mNormal = std::make_shared<GLframework::Texture>("Texture/FFFFFF.png", 2);
	setMaterialType(MaterialType::PhongNormalMaterial);
}

PhongNormalMaterial::~PhongNormalMaterial()
{
	
}


