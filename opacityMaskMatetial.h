#pragma once

#include "material.h"
#include "texture.h"
namespace GLframework
{
	// default setting : ColorBlendState = true
	class OpacityMaskMaterial : public Material
	{
	public:
		OpacityMaskMaterial();
		OpacityMaskMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(diffuse), mShiness(shiness)
		{
			this->setMaterialType(MaterialType::OpacityMaskMaterial);
			this->setColorBlendState(true);
			this->setDepthWrite(false);
		}
		~OpacityMaskMaterial();


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mOpacityrMask{ nullptr };
		float mShiness{ 10.0f };
	};
}