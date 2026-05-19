#pragma once

#include <memory>

#include "framework/shader.h"
#include "framebuffer/framebuffer.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class Bloom;

	class PostProcessPass
	{
	public:
		void resolveMultisample(
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst
		) const;

		void renderScreenComposite(
			const std::shared_ptr<Mesh>& screenQuad,
			const std::shared_ptr<Shader>& shader,
			unsigned int width,
			unsigned int height,
			unsigned int targetFbo = 0
		) const;

		void extractBloomBright(
			const std::shared_ptr<Bloom>& bloom,
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst
		) const;

		void blurBloom(
			const std::shared_ptr<Bloom>& bloom,
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& ping,
			const std::shared_ptr<Framebuffer>& pong,
			int iterations = 6
		) const;
	};
}
