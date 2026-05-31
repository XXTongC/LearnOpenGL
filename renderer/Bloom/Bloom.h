#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	class Framebuffer;
	class Geometry;
	class Shader;
	class Texture;

	class Bloom
	{
	private:
		int mWidth{ 0 };
		int mHeight{ 0 };
		// �²���ͼƬ����
		int mMipLevels{ 0 };
		std::shared_ptr<Shader> mExtractBrightShader{ nullptr };
		std::shared_ptr<Geometry> mQuad{ nullptr };
		std::vector<std::shared_ptr<Framebuffer>> mDownSamples{};
		std::vector<std::shared_ptr<Framebuffer>> mUpSamples{};
		
	public:
		Bloom(int width,int height, int min_Resolution = 32);
		~Bloom();
		void extractBright(
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst,
			float threshold
		) const;
		void blurPingPong(
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& ping,
			const std::shared_ptr<Framebuffer>& pong,
			int iterations = 6
		) const;

	private:
		void drawTextureToTarget(
			const std::shared_ptr<Texture>& source,
			const std::shared_ptr<Framebuffer>& target,
			bool horizontal
		) const;

	private:
		std::shared_ptr<Shader> mBlurShader{ nullptr };
	};
}
