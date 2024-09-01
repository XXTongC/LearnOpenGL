#pragma once

#include "material.h"
#include "texture.h"

namespace GLframework
{
	class PhongMaterial: public Materal
	{
	public:
		PhongMaterial();
		PhongMaterial(Texture* diffuse,float shiness)
			:mDiffuse(diffuse), mShiness(shiness)
		{ 
			mType = MaterialType::PhongMaterial;
		}
		~PhongMaterial();
	public:
		Texture* mDiffuse{ nullptr };
		float mShiness{ 1.0f };
	};
}
