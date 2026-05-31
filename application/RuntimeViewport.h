#pragma once

#include <memory>

class Camera;

namespace GLframework
{
	class FrameRenderTargets;
	class ScreenMaterial;
}

namespace GL_RUNTIME
{
	struct RuntimeResizeContext
	{
		int* width{ nullptr };
		int* height{ nullptr };
		Camera* camera{ nullptr };
		GLframework::FrameRenderTargets* frameRenderTargets{ nullptr };
		std::shared_ptr<GLframework::ScreenMaterial> screenMaterial{ nullptr };
	};

	struct RuntimeResizeResult
	{
		bool accepted{ false };
		bool renderTargetsRebuilt{ false };
	};

	class RuntimeViewport
	{
	public:
		static bool isValidSize(int width, int height);
		static float aspectRatio(int width, int height);
		static void applyViewport(int width, int height);
		static void syncCameraAspect(Camera* camera, int width, int height);
		static void syncPostProcessInputTextures(
			const std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial,
			const GLframework::FrameRenderTargets& frameRenderTargets
		);
		static RuntimeResizeResult applyResize(int width, int height, const RuntimeResizeContext& context);
	};
}
