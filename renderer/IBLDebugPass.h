#pragma once

#include <memory>

#include "mesh/mesh.h"

namespace GLframework
{
	class EnvironmentRenderTargets;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	class IBLDebugPass
	{
	public:
		int render(
			const EnvironmentRenderTargets& environmentTargets,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary
		);

	private:
		void ensureDebugQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mDebugQuad{ nullptr };
	};
}
