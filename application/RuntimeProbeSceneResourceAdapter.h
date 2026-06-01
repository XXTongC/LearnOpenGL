#pragma once

#include <memory>

namespace GLframework
{
	class Geometry;
	class Object;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState;

	class RuntimeProbeSceneResourceAdapter
	{
	public:
		static bool hasOffScreenSceneAndRenderer(const RuntimeRenderResourceState& renderResources);

		static std::shared_ptr<GLframework::Geometry> createPbrSphereGeometry(
			const RuntimeRenderResourceState& renderResources,
			float radius,
			int sectorCount,
			int stackCount
		);

		static std::shared_ptr<GLframework::Geometry> createPbrPlaneGeometry(
			const RuntimeRenderResourceState& renderResources,
			float width,
			float height
		);

		static void addOffScreenSceneChild(
			const RuntimeRenderResourceState& renderResources,
			const std::shared_ptr<GLframework::Object>& object
		);
	};
}
