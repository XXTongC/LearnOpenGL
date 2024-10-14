#pragma once

#include "material.h"
#include "texture.h"
namespace GLframework
{
	class PhongMaterial: public Material
	{
	public:
		PhongMaterial();
		PhongMaterial(std::shared_ptr<Texture> diffuse,float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{ 
			setMaterialType(MaterialType::PhongMaterial);
		}
		~PhongMaterial();
		

	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };
	};
}
