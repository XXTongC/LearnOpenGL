#pragma once

#include <memory>
#include <vector>

#include "MaterialBindingContext.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class PBRGBufferRenderTargets;
	class ShaderLibrary;

	struct PBRGBufferPassStats
	{
		int drawCalls{ 0 };
		int targetWidth{ 0 };
		int targetHeight{ 0 };
		bool ready{ false };
	};

	class PBRGBufferPass
	{
	public:
		PBRGBufferPassStats render(
			const std::vector<std::shared_ptr<Mesh>>& meshes,
			const MaterialBindingContext& context,
			ShaderLibrary& shaderLibrary,
			PBRGBufferRenderTargets& targets
		);
	};
}
