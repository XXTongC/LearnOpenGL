#pragma once

#include <memory>
#include <string>

#include "../third_party/glm/fwd.hpp"

namespace GLframework
{
	class Material;
	class Object;
	class Renderer;
}

namespace GL_APPLICATION
{
	
	class AssimpInstanceLoader
	{
	public:
		static std::shared_ptr<GLframework::Object> load(
			const std::string& path, 
			std::shared_ptr<GLframework::Renderer> renderer,
			const unsigned int& instanceCount
		);
		static void setInstanceMatrix(std::shared_ptr<GLframework::Object> obj, unsigned int index, const glm::mat4& matrix);
		static void updateInstanceMatrix(std::shared_ptr<GLframework::Object> obj);
		static void setInstanceMaterial(std::shared_ptr<GLframework::Object> obj, std::shared_ptr<GLframework::Material> material);
	};
}
