#include "screenMaterial.h"

using namespace GLframework;

const std::shared_ptr<Texture>& ScreenMaterial::screenTexture() const
{
	return mScreenTexture;
}

const std::shared_ptr<Texture>& ScreenMaterial::bloomTexture() const
{
	return mBloomTexture;
}

const std::shared_ptr<Texture>& ScreenMaterial::depthStencilTexture() const
{
	return mDepthStencilTexture;
}
