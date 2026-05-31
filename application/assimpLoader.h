#pragma once

#include <memory>
#include <string>

#include "AssimpMaterialImporter.h"

namespace GLframework
{
	class Object;
	class Renderer;
}

namespace GL_APPLICATION
{
	class AssimpLoader
	{
	public:
		static std::shared_ptr<GLframework::Object> load(
			const std::string& path, 
			std::shared_ptr<GLframework::Renderer> renderer
		);
		static std::shared_ptr<GLframework::Object> load(
			const std::string& path,
			std::shared_ptr<GLframework::Renderer> renderer,
			const AssimpMaterialImportOptions& materialOptions
		);
		static std::shared_ptr<GLframework::Object> loadPBR(
			const std::string& path,
			std::shared_ptr<GLframework::Renderer> renderer
		);
	};
}
