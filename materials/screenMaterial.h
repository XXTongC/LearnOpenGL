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
		void setInputTextures(
			std::shared_ptr<Texture> screenTexture,
			std::shared_ptr<Texture> bloomTexture,
			std::shared_ptr<Texture> depthStencilTexture
		);
		ScreenMaterialInputTextures inputTextures() const;
	
	private:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mBloomTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
	};
}
