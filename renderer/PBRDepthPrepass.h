#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "mesh/mesh.h"
#include "renderer/ShaderLibrary.h"

namespace GLframework
{
	class PBRDepthPrepass
	{
	public:
		int render(
			const std::vector<std::shared_ptr<Mesh>>& pbrOpacityObjects,
			Camera* camera,
			const ShaderLibrary& shaderLibrary
		) const;

	private:
		bool renderObject(
			const std::shared_ptr<Mesh>& mesh,
			Camera* camera,
			const std::shared_ptr<Shader>& shader
		) const;

		void drawMesh(const std::shared_ptr<Mesh>& mesh) const;
	};
}
