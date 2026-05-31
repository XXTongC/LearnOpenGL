#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	class Mesh;

	class ShadowMeshDraw
	{
	public:
		static bool isPostProcessPass(const std::vector<std::shared_ptr<Mesh>>& meshes);
		static bool draw(const std::shared_ptr<Mesh>& mesh);
	};
}
