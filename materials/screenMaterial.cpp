#include "screenMaterial.h"

#include <utility>

using namespace GLframework;

void ScreenMaterial::setInputTextures(
	std::shared_ptr<Texture> screenTexture,
	std::shared_ptr<Texture> bloomTexture,
	std::shared_ptr<Texture> depthStencilTexture
)
{
	mScreenTexture = std::move(screenTexture);
	mBloomTexture = std::move(bloomTexture);
	mDepthStencilTexture = std::move(depthStencilTexture);
}

ScreenMaterialInputTextures ScreenMaterial::inputTextures() const
{
	return ScreenMaterialInputTextures{
		&mScreenTexture,
		&mBloomTexture,
		&mDepthStencilTexture
	};
}
