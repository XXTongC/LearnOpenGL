#pragma once

#include <memory>

namespace GLframework
{
	class Mesh;

	class MeshDraw
	{
	public:
		static bool drawIndexed(const std::shared_ptr<Mesh>& mesh);
	};
}
