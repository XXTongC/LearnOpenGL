#include "phongMaterial.h"

#include <utility>

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

void PhongMaterial::setSurface(PhongSurfaceInput surface)
{
	mDiffuse = std::move(surface.diffuseTexture);
	mSpecularMask = std::move(surface.specularMaskTexture);
	mShiness = surface.shininess;
}

void PhongMaterial::setDiffuseTexture(std::shared_ptr<Texture> diffuse)
{
	mDiffuse = std::move(diffuse);
}

void PhongMaterial::setSpecularMaskTexture(std::shared_ptr<Texture> specularMask)
{
	mSpecularMask = std::move(specularMask);
}

void PhongMaterial::setShininess(float shininess)
{
	mShiness = shininess;
}

PhongSurfaceRuntimeState PhongMaterial::surfaceState() const
{
	return PhongSurfaceRuntimeState{
		&mDiffuse,
		&mSpecularMask,
		mShiness
	};
}

PhongSurfaceEditControls PhongMaterial::surfaceEditControls()
{
	return PhongSurfaceEditControls{
		&mShiness,
		&mDiffuse,
		&mSpecularMask
	};
}
