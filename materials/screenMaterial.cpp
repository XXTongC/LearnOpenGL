#include "screenMaterial.h"

using namespace GLframework;

ScreenMaterialInputTextures ScreenMaterial::inputTextures() const
{
	return ScreenMaterialInputTextures{
		&mScreenTexture,
		&mBloomTexture,
		&mDepthStencilTexture
	};
}
