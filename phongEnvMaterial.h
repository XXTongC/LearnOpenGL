#pragma once
#include "material.h"
#include "texture.h"

namespace GLframework
{
	class PhongEnvMaterial : public Material
	{
	public:
		PhongEnvMaterial()
		{
			mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
			this->setMaterialType(MaterialType::PhongEnvMaterial);
		}
		~PhongEnvMaterial() {}
	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mEnv{nullptr};
		float mShiness{ 10.0f };

	};
}
