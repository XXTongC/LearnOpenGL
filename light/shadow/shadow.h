#pragma once
#include "../../core.h"
#include "../../framebuffer.h"
#include "../../camera.h"

namespace GLframework
{
	class Shadow
	{
	public:
		Shadow(){}
		virtual ~Shadow(){}
		virtual void setRenderTargetSize(int width, int height) = 0;
		float mBias{ 0.0003f };
		float mPcfRadius{ 0.1f };
		float mDiskTightness{ 1.0f };



	public:
		std::shared_ptr<Camera> mCamera{ nullptr };
		std::shared_ptr<Framebuffer> mRenderTarget{ nullptr };


	};
}
