#include "MaterialInspector.h"

#include <sstream>

#include "../../framework/texture.h"
#include "../../materials/material.h"
#include "PropertyInspector.h"

namespace GL_EDITOR
{
	std::string getMaterialTypeName(GLframework::MaterialType type)
	{
		switch (type)
		{
		case GLframework::MaterialType::PhongMaterial: return "PhongMaterial";
		case GLframework::MaterialType::PBRMaterial: return "PBRMaterial";
		case GLframework::MaterialType::WhiteMaterial: return "WhiteMaterial";
		case GLframework::MaterialType::DepthMaterial: return "DepthMaterial";
		case GLframework::MaterialType::OpacityMaskMaterial: return "OpacityMaskMaterial";
		case GLframework::MaterialType::ScreenMaterial: return "ScreenMaterial";
		case GLframework::MaterialType::CubeMaterial: return "CubeMaterial";
		case GLframework::MaterialType::CubeSphereMaterial: return "CubeSphereMaterial";
		case GLframework::MaterialType::PhongEnvMaterial: return "PhongEnvMaterial";
		case GLframework::MaterialType::PhongEnvSphereMaterial: return "PhongEnvSphereMaterial";
		case GLframework::MaterialType::PhongInstanceMaterial: return "PhongInstanceMaterial";
		case GLframework::MaterialType::PhongNormalMaterial: return "PhongNormalMaterial";
		case GLframework::MaterialType::GrassInstanceMaterial: return "GrassInstanceMaterial";
		case GLframework::MaterialType::PhongParallaxMaterial: return "PhongParallaxMaterial";
		case GLframework::MaterialType::PhongShadowMaterial: return "PhongShadowMaterial";
		case GLframework::MaterialType::PhongCSMShadowMaterial: return "PhongCSMShadowMaterial";
		case GLframework::MaterialType::PhongPointShadowMaterial: return "PhongPointShadowMaterial";
		default: return "UnknownMaterial";
		}
	}

	std::string describeTexture(const std::shared_ptr<GLframework::Texture>& texture)
	{
		if (!texture) return "None";

		std::ostringstream stream;
		stream
			<< "Unit " << texture->getUnit()
			<< ", " << texture->getWidth() << "x" << texture->getHeight()
			<< ", Target " << texture->getTextureTarget();
		return stream.str();
	}

	bool drawMaterialInspector(GLframework::Material& material)
	{
		PropertyBuilder builder;
		material.visitEditableProperties(builder);
		return drawProperties(builder);
	}
}
