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
		const std::shared_ptr<Texture>& screenTexture() const;
		const std::shared_ptr<Texture>& bloomTexture() const;
		const std::shared_ptr<Texture>& depthStencilTexture() const;
	
	public:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
		std::shared_ptr<Texture> mBloomTexture{ nullptr };
	};
}
