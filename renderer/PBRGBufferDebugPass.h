#pragma once

#include <memory>

#include "mesh/mesh.h"

namespace GLframework
{
	class PBRGBufferRenderTargets;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	class PBRGBufferDebugPass
	{
	public:
		int render(
			const PBRGBufferRenderTargets& targets,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary
		);

	private:
		void ensureDebugQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mDebugQuad{ nullptr };
	};
}
