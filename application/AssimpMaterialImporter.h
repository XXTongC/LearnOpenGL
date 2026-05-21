#pragma once

#include <memory>
#include <string>

#include "materials/material.h"
#include "third_party/assimp/scene.h"

namespace GL_APPLICATION
{
	enum class AssimpMaterialImportMode
	{
		LegacyPhong,
		PBRMetallicRoughness,
	};

	struct AssimpMaterialImportOptions
	{
		AssimpMaterialImportMode mode{ AssimpMaterialImportMode::LegacyPhong };
		bool enablePbrIbl{ true };
		float defaultAlphaCutoff{ 0.5f };
		bool useDefaultPhongTextures{ true };
	};

	class AssimpMaterialImporter
	{
	public:
		static std::shared_ptr<GLframework::Material> createMaterial(
			const aiMesh* mesh,
			const aiScene* scene,
			const std::string& rootPath,
			const AssimpMaterialImportOptions& options
		);
	};
}
