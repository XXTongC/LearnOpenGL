#pragma once

#include <memory>

#include "mesh/mesh.h"
#include "renderer/PBRDeferredClusteredLightGrid.h"

namespace GLframework
{
	struct MaterialBindingContext;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	struct PBRDeferredClusteredLightDebugPassStats
	{
		int drawCalls{ 0 };
		PBRDeferredClusteredLightGridStats gridStats{};
	};

	class PBRDeferredClusteredLightDebugPass
	{
	public:
		PBRDeferredClusteredLightDebugPassStats render(
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary,
			unsigned int targetWidth,
			unsigned int targetHeight
		);

	private:
		void ensureDebugQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mDebugQuad{ nullptr };
		PBRDeferredClusteredLightGrid mClusteredLightGrid{};
	};
}
