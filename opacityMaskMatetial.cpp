#include "opacityMaskMatetial.h"

using namespace GLframework;

OpacityMaskMaterial::OpacityMaskMaterial()
{
	mOpacityrMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setColorBlendState(true);
	setMaterialType(MaterialType::OpacityMaskMaterial);
	this->setDepthWrite(false);
}

OpacityMaskMaterial::~OpacityMaskMaterial()
{
	
}

