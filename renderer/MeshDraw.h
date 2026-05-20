#pragma once

#include <memory>

#include "mesh/mesh.h"

namespace GLframework
{
	class MeshDraw
	{
	public:
		static bool drawIndexed(const std::shared_ptr<Mesh>& mesh);
	};
}
