#pragma once

#include <memory>

#include "framebuffer/framebuffer.h"

namespace GLframework
{
	class FrameRenderTargets
	{
	public:
		void initialize(unsigned int width, unsigned int height, unsigned int samples = 4);

		bool isInitialized() const;
		unsigned int getSceneFbo() const;
		unsigned int getWidth() const;
		unsigned int getHeight() const;
		unsigned int getSamples() const;

		const std::shared_ptr<Framebuffer>& getMultisample() const;
		const std::shared_ptr<Framebuffer>& getResolved() const;
		const std::shared_ptr<Framebuffer>& getBloomBright() const;
		const std::shared_ptr<Framebuffer>& getBloomPing() const;
		const std::shared_ptr<Framebuffer>& getBloomPong() const;
		std::shared_ptr<Texture> getResolvedColorAttachment() const;
		std::shared_ptr<Texture> getBloomBrightColorAttachment() const;

	private:
		unsigned int mWidth{ 0 };
		unsigned int mHeight{ 0 };
		unsigned int mSamples{ 0 };
		std::shared_ptr<Framebuffer> mMultisample{ nullptr };
		std::shared_ptr<Framebuffer> mResolved{ nullptr };
		std::shared_ptr<Framebuffer> mBloomBright{ nullptr };
		std::shared_ptr<Framebuffer> mBloomPing{ nullptr };
		std::shared_ptr<Framebuffer> mBloomPong{ nullptr };
	};
}
