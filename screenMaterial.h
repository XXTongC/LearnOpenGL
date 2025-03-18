#pragma once

#include "material.h"
#include "texture.h"
namespace GLframework
{
	class ScreenMaterial : public Material
	{
	public:
		ScreenMaterial()
		{
			setMaterialType(MaterialType::ScreenMaterial);
		}
		~ScreenMaterial(){}
	
	public:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
	};
}
