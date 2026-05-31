#pragma once

#include <memory>

namespace GLframework
{
	class EnvironmentRenderTargets;
	class Mesh;
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
