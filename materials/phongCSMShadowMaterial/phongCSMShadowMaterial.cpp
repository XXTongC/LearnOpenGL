#include "phongCSMShadowMaterial.h"

#include <utility>

using namespace GLframework;

void PhongCSMShadowMaterial::setSurface(PhongSurfaceInput surface)
{
	mDiffuse = std::move(surface.diffuseTexture);
	mSpecularMask = std::move(surface.specularMaskTexture);
	mShiness = surface.shininess;
}

void PhongCSMShadowMaterial::setDiffuseTexture(std::shared_ptr<Texture> diffuse)
{
	mDiffuse = std::move(diffuse);
}

void PhongCSMShadowMaterial::setSpecularMaskTexture(std::shared_ptr<Texture> specularMask)
{
	mSpecularMask = std::move(specularMask);
}

void PhongCSMShadowMaterial::setShininess(float shininess)
{
	mShiness = shininess;
}

PhongSurfaceRuntimeState PhongCSMShadowMaterial::surfaceState() const
{
	return PhongSurfaceRuntimeState{
		&mDiffuse,
		&mSpecularMask,
		mShiness
	};
}

PhongSurfaceEditControls PhongCSMShadowMaterial::surfaceEditControls()
{
	return PhongSurfaceEditControls{
		&mShiness,
		&mDiffuse,
		&mSpecularMask
	};
}
