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

PhongSurfaceEditControls PhongPointShadowMaterial::surfaceEditControls()
{
	return PhongSurfaceEditControls{
		&mShiness,
		&mDiffuse,
		&mSpecularMask
	};
}
