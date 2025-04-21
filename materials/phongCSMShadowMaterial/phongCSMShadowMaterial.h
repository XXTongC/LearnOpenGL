#pragma once
#include "../../material.h"
#include "../../texture.h"

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


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };

	};
}
