#pragma once

#include <memory>

#include "mesh/mesh.h"
#include "renderer/PBRDeferredTiledLightGrid.h"

namespace GLframework
{
	struct MaterialBindingContext;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	struct PBRDeferredTiledLightDebugPassStats
	{
		int drawCalls{ 0 };
		PBRDeferredTiledLightGridStats gridStats{};
	};

	class PBRDeferredTiledLightDebugPass
	{
	public:
		PBRDeferredTiledLightDebugPassStats render(
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary,
			unsigned int targetWidth,
			unsigned int targetHeight
		);

	private:
		void ensureDebugQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mDebugQuad{ nullptr };
		PBRDeferredTiledLightGrid mTiledLightGrid{};
	};
}
