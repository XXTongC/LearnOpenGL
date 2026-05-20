#pragma once

#include <memory>

#include "MaterialBindingContext.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class PBRGBufferRenderTargets;
	struct RendererFramePassProfile;
	class Shader;
	class ShaderLibrary;

	class PBRDeferredLightingPass
	{
	public:
		int render(
			const PBRGBufferRenderTargets& targets,
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile,
			ShaderLibrary& shaderLibrary
		);

	private:
		void bindFrameUniforms(
			const std::shared_ptr<Shader>& shader,
			const MaterialBindingContext& context,
			const RendererFramePassProfile& profile
		) const;
		void bindGBufferTextures(
			const std::shared_ptr<Shader>& shader,
			const PBRGBufferRenderTargets& targets
		) const;
		void ensureLightingQuad(const std::shared_ptr<Shader>& shader);

		std::shared_ptr<Mesh> mLightingQuad{ nullptr };
	};
}
