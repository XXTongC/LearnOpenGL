#pragma once

#include "material.h"
#include "renderer/PostProcessSettings.h"
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
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder) override;
	
	public:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
		std::shared_ptr<Texture> mBloomTexture{ nullptr };
		PostProcessSettings mSettings{};
	};
}
