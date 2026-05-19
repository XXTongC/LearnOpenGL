#pragma once
#include "../../core.h"
#include "../../framebuffer/framebuffer.h"
#include "../../framework/geometry.h"
#include "../../framework/shader.h"
namespace GLframework
{
	class Bloom
	{
	private:
		int mWidth{ 0 };
		int mHeight{ 0 };
		// �²���ͼƬ����
		int mMipLevels{ 0 };
		float mThreshold{ 0.0f };
		std::shared_ptr<Shader> mExtractBrightShader{ nullptr };
		std::shared_ptr<Geometry> mQuad{ nullptr };
		std::vector<std::shared_ptr<Framebuffer>> mDownSamples{};
		std::vector<std::shared_ptr<Framebuffer>> mUpSamples{};
		
	public:
		Bloom(int width,int height, int min_Resolution = 32);
		~Bloom();
		void extractBright(
			const std::shared_ptr<Framebuffer>& src,
			const std::shared_ptr<Framebuffer>& dst
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
