#pragma once


#include "../material.h"
#include "../MaterialEditControls.h"
#include "../../framework/texture.h"
namespace GLframework
{
	class PhongPointShadowMaterial : public Material
	{
	public:
		PhongPointShadowMaterial();
		PhongPointShadowMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongPointShadowMaterial);
		}
		~PhongPointShadowMaterial();
		void setSurface(PhongSurfaceInput surface);
		void setDiffuseTexture(std::shared_ptr<Texture> diffuse);
		void setSpecularMaskTexture(std::shared_ptr<Texture> specularMask);
		void setShininess(float shininess);
		PhongSurfaceRuntimeState surfaceState() const;
		PhongSurfaceEditControls surfaceEditControls();


	private:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };



	};
}
