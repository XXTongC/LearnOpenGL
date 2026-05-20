#include "PBRShadowResourceBinder.h"

#include "light/directionalLight.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
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

	if (getCsmLayerCount(context) <= 0)
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

int PBRShadowResourceBinder::getCsmLayerCount(const MaterialBindingContext& context)
{
	if (context.camera == nullptr || !context.dirLight || !context.dirLight->getShadow())
	{
		return 0;
	}

	const auto dirCSMShadow = std::dynamic_pointer_cast<DirectionalLightCSMShadow>(context.dirLight->getShadow());
	return dirCSMShadow ? dirCSMShadow->getLayerCount() : 0;
}
