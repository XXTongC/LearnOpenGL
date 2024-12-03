#include "grassInstanceMaterial/grassInstanceMaterial.h"

using namespace GLframework;

GrassInstanceMaterial::GrassInstanceMaterial()
{
	//setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::GrassInstanceMaterial);
}

GrassInstanceMaterial::~GrassInstanceMaterial()
{

}


