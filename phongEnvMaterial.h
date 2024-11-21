#pragma once
#include "material.h"
#include "texture.h"

namespace GLframework
{
	class PhongEnvMaterial : Material
	{
	public:
		PhongEnvMaterial()
		{
			setMaterialType(MaterialType::PhongEnvMaterial);
		}
		~PhongEnvMaterial() {}
	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mEnv{nullptr};
		float mShiness{ 1.0f };

	};
}
