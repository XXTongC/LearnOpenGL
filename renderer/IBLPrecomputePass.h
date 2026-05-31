#pragma once

#include <memory>

namespace GLframework
{
	class EnvironmentRenderTargets;
	class Mesh;
	class Shader;
	class ShaderLibrary;
	class Texture;

	class IBLPrecomputePass
	{
	public:
		bool captureEnvironmentMap(
			const std::shared_ptr<Texture>& equirectangularMap,
			EnvironmentRenderTargets& targets,
			const std::shared_ptr<Mesh>& captureCube,
			const ShaderLibrary& shaderLibrary
		) const;

		bool computeIrradianceMap(
			EnvironmentRenderTargets& targets,
			const std::shared_ptr<Mesh>& captureCube,
			const ShaderLibrary& shaderLibrary
		) const;

		bool computePrefilterMap(
			EnvironmentRenderTargets& targets,
			const std::shared_ptr<Mesh>& captureCube,
			const ShaderLibrary& shaderLibrary
		) const;

		bool computeBrdfLut(
			EnvironmentRenderTargets& targets,
			const std::shared_ptr<Mesh>& screenQuad,
			const ShaderLibrary& shaderLibrary
		) const;

	private:
		bool renderCubemapFaces(
			EnvironmentRenderTargets& targets,
			const std::shared_ptr<Texture>& outputCubemap,
			const std::shared_ptr<Mesh>& captureCube,
			const std::shared_ptr<Shader>& shader,
			unsigned int captureSize,
			unsigned int mipLevel = 0
		) const;
	};
}
