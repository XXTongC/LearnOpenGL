#pragma once

#include <memory>
#include "../material.h"
#include "../texture.h"

namespace GLframework
{
	class Texture;

	class GrassInstanceMaterial : public Material
	{
	public:
		GrassInstanceMaterial();
		GrassInstanceMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::GrassInstanceMaterial);
		}
		~GrassInstanceMaterial();


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };
	};
}
