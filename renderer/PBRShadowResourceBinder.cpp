#include "PBRShadowResourceBinder.h"

#include "light/directionalLight.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	constexpr int pbrCsmShadowTextureUnit = 8;
}

bool PBRShadowResourceBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const MaterialBindingContext& context
)
{
	if (!shader)
	{
		return false;
	}

	if (context.camera == nullptr || !context.dirLight || !context.dirLight->getShadow())
	{
		shader->setInt("csmLayerCount", 0);
		return false;
	}

	ShadowResourceBinder::bindCSMShadowResources(
		shader,
		context.camera,
		context.dirLight,
		pbrCsmShadowTextureUnit
	);
	return true;
}
