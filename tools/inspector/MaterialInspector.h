#pragma once

#include <memory>
#include <string>

namespace GLframework
{
	enum class MaterialType;
	class Material;
	class Texture;
}

namespace GL_EDITOR
{
	std::string getMaterialTypeName(GLframework::MaterialType type);
	std::string describeTexture(const std::shared_ptr<GLframework::Texture>& texture);
	bool drawMaterialInspector(GLframework::Material& material);
}
