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
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder) override;
	
	public:
		std::shared_ptr<Texture> mScreenTexture{ nullptr };
		std::shared_ptr<Texture> mDepthStencilTexture{ nullptr };
		float mExposure = 1.0f;
	};
}
