#include "phongParallaxMaterial.h"
using namespace GLframework;

PhongParallaxMaterial::PhongParallaxMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/FFFFFF.png", 1);
	mNormal = std::make_shared<GLframework::Texture>("Texture/FFFFFF.png", 2);
	mParallaxMap = std::make_shared<GLframework::Texture>("Texture/FFFFFF.png", 3);
	setMaterialType(MaterialType::PhongParallaxMaterial);
}

PhongParallaxMaterial::~PhongParallaxMaterial()
{

}

