#include "phongPointShadowMaterial.h"

#include <utility>

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

void PhongPointShadowMaterial::setSurface(PhongSurfaceInput surface)
{
	mDiffuse = std::move(surface.diffuseTexture);
	mSpecularMask = std::move(surface.specularMaskTexture);
	mShiness = surface.shininess;
}

void PhongPointShadowMaterial::setDiffuseTexture(std::shared_ptr<Texture> diffuse)
{
	mDiffuse = std::move(diffuse);
}

void PhongPointShadowMaterial::setSpecularMaskTexture(std::shared_ptr<Texture> specularMask)
{
	mSpecularMask = std::move(specularMask);
}

void PhongPointShadowMaterial::setShininess(float shininess)
{
	mShiness = shininess;
}

PhongSurfaceRuntimeState PhongPointShadowMaterial::surfaceState() const
{
	return PhongSurfaceRuntimeState{
		&mDiffuse,
		&mSpecularMask,
		mShiness
	};
}

PhongSurfaceEditControls PhongPointShadowMaterial::surfaceEditControls()
{
	return PhongSurfaceEditControls{
		&mShiness,
		&mDiffuse,
		&mSpecularMask
	};
}
