#pragma once
#include "../../core.h"
#include "../../framebuffer.h"
namespace GLframework
{
	class Bloom
	{
	private:
		std::vector<std::shared_ptr<Framebuffer>> mDownSamples{};
		std::vector<std::shared_ptr<Framebuffer>> mUpSamples{};
		int mWidth{ 0 };
		int mHeight{ 0 };
		// 下采样图片数量
		int mMipLevels{ 0 };
	public:
		Bloom(int width,int height, int min_Resolution = 32);
		~Bloom();
	};
}
