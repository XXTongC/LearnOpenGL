#pragma once

#include <memory>

#include "third_party/glm/glm.hpp"

namespace GLframework
{
	class Mesh;
	class Shader;

	class PBRAlphaShadowBinder
	{
	public:
		static bool isAlphaMaskedPbrMesh(const std::shared_ptr<Mesh>& mesh);
		static bool bindDirectional(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<Mesh>& mesh,
			const glm::mat4& lightMatrix
		);
		static bool bindPoint(
			const std::shared_ptr<Shader>& shader,
			const std::shared_ptr<Mesh>& mesh,
			const glm::mat4& lightSpaceMatrix,
			const glm::vec3& lightPosition,
			float farPlane
		);
	};
}
