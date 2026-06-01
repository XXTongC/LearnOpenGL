#pragma once
#include "../material.h"
#include "../MaterialEditControls.h"
#include "../../framework/texture.h"

namespace GLframework
{

	class PhongCSMShadowMaterial: public Material
	{
	public:
		PhongCSMShadowMaterial()
		{
			mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
			this->setMaterialType(GLframework::MaterialType::PhongCSMShadowMaterial);
		}

		~PhongCSMShadowMaterial() override{}
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
