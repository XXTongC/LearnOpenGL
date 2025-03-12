#pragma once

#include "../material.h"
#include "../texture.h"
namespace GLframework
{
	class PhongParallaxMaterial : public Material
	{
	public:
		PhongParallaxMaterial();
		PhongParallaxMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongParallaxMaterial);
		}
		~PhongParallaxMaterial();

	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mNormal{ nullptr };
		std::shared_ptr<Texture> mParallaxMap{ nullptr };
		float mHeightScale{ 0.1f };
		float mShiness{ 10.0f };
		int mLayerNum{ 10 };
	};
}
