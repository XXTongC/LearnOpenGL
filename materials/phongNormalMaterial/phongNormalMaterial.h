#pragma once

#include "../../material.h"
#include "../../texture.h"
namespace GLframework
{
	class PhongNormalMaterial : public Material
	{
	public:
		PhongNormalMaterial();
		PhongNormalMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongNormalMaterial);
		}
		~PhongNormalMaterial();


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mNormal{nullptr};
		float mShiness{ 10.0f };
	};
}
