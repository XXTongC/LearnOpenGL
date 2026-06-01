#pragma once

#include "material.h"
#include "MaterialEditControls.h"
#include "texture.h"
namespace GLframework
{
	class PhongMaterial: public Material
	{
	public:
		PhongMaterial();
		PhongMaterial(std::shared_ptr<Texture> diffuse,float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongMaterial);
		}
		~PhongMaterial();
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
