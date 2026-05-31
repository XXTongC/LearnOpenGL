#pragma once

#include <memory>

namespace GLframework
{
	class Bloom;
	class Framebuffer;
	class Mesh;
	struct PostProcessSettings;
	class Shader;

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
			const PostProcessSettings& settings,
			unsigned int width,
			unsigned int height,
			unsigned int targetFbo = 0
		) const;

		void extractBloomBright(
			const std::shared_ptr<Bloom>& bloom,
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst,
			float threshold
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
