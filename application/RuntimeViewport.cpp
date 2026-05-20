#include "RuntimeViewport.h"

#include "GL_ERROR_FIND.h"
#include "camera/perspectivecamera.h"

using namespace GL_RUNTIME;

bool RuntimeViewport::isValidSize(int width, int height)
{
	return width > 0 && height > 0;
}

float RuntimeViewport::aspectRatio(int width, int height)
{
	if (!isValidSize(width, height))
	{
		return 1.0f;
	}

	return static_cast<float>(width) / static_cast<float>(height);
}

void RuntimeViewport::applyViewport(int width, int height)
{
	if (!isValidSize(width, height))
	{
		return;
	}

	GL_CALL(glViewport(0, 0, width, height));
}

void RuntimeViewport::syncCameraAspect(Camera* camera, int width, int height)
{
	if (camera == nullptr || !isValidSize(width, height))
	{
		return;
	}

	if (auto perspectiveCamera = dynamic_cast<PerspectiveCamera*>(camera))
	{
		perspectiveCamera->mAspect = aspectRatio(width, height);
	}
}

void RuntimeViewport::syncPostProcessInputTextures(
	const std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial,
	const GLframework::FrameRenderTargets& frameRenderTargets
)
{
	if (screenMaterial == nullptr)
	{
		return;
	}

	screenMaterial->mScreenTexture = frameRenderTargets.getResolvedColorAttachment();
	screenMaterial->mDepthStencilTexture = frameRenderTargets.getResolvedDepthStencilAttachment();
	screenMaterial->mBloomTexture = frameRenderTargets.getBloomPongColorAttachment();
}

RuntimeResizeResult RuntimeViewport::applyResize(int width, int height, const RuntimeResizeContext& context)
{
	RuntimeResizeResult result{};
	if (!isValidSize(width, height))
	{
		return result;
	}

	result.accepted = true;
	if (context.width != nullptr)
	{
		*context.width = width;
	}

	if (context.height != nullptr)
	{
		*context.height = height;
	}

	applyViewport(width, height);
	syncCameraAspect(context.camera, width, height);

	if (context.frameRenderTargets != nullptr)
	{
		result.renderTargetsRebuilt = context.frameRenderTargets->resize(
			static_cast<unsigned int>(width),
			static_cast<unsigned int>(height)
		);
		if (result.renderTargetsRebuilt)
		{
			syncPostProcessInputTextures(context.screenMaterial, *context.frameRenderTargets);
		}
	}

	return result;
}
