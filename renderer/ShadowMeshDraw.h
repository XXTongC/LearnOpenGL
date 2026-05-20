#pragma once

#include <memory>
#include <vector>

#include "materials/material.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class ShadowMeshDraw
	{
	public:
		static bool isPostProcessPass(const std::vector<std::shared_ptr<Mesh>>& meshes);
		static void draw(const std::shared_ptr<Mesh>& mesh);
	};
}
