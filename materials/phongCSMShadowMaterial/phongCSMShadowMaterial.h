#pragma once
#include "../material.h"
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
		float* shininessControl();
		const std::shared_ptr<Texture>& diffuseTexture() const;
		const std::shared_ptr<Texture>& specularMaskTexture() const;


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };

	};
}
