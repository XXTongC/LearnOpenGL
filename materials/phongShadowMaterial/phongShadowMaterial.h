#pragma once


#include "../../material.h"
#include "../../texture.h"
namespace GLframework
{
	class PhongShadowMaterial : public Material
	{
	public:
		PhongShadowMaterial();
		PhongShadowMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongShadowMaterial);
		}
		~PhongShadowMaterial();


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };
	};
}
