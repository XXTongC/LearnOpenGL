#pragma once


#include "../material.h"
#include "../../framework/texture.h"
namespace GLframework
{
	class PhongPointShadowMaterial : public Material
	{
	public:
		PhongPointShadowMaterial();
		PhongPointShadowMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setMaterialType(MaterialType::PhongPointShadowMaterial);
		}
		~PhongPointShadowMaterial();
		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder) override;


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		float mShiness{ 10.0f };



	};
}
