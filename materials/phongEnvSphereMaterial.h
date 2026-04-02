#pragma once
#include "material.h"
#include "texture.h"

namespace GLframework
{
	class PhongEnvSphereMaterial : public Material
	{
	public:
		PhongEnvSphereMaterial()
		{
			mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
			this->setMaterialType(MaterialType::PhongEnvSphereMaterial);
		}
		~PhongEnvSphereMaterial() {}
	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mEnv{ nullptr };
		float mShiness{ 10.0f };

	};
}
