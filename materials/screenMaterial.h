#pragma once

#include "material.h"
#include "MaterialEditControls.h"
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
		ScreenMaterialInputTextures inputTextures() const;
	
	public:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
		std::shared_ptr<Texture> mBloomTexture{ nullptr };
	};
}
