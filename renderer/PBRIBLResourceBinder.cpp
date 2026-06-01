#include "PBRIBLResourceBinder.h"

#include "framework/shader.h"
#include "framework/texture.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "renderer/EnvironmentRenderTargets.h"

using namespace GLframework;

namespace
{
	void bindTexture(const std::shared_ptr<Shader>& shader, const char* samplerName, const std::shared_ptr<Texture>& texture)
	{
		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}
}

bool PBRIBLResourceBinder::canUseIBL(
	const std::shared_ptr<PBRMaterial>& material,
	const EnvironmentRenderTargets* environmentTargets
)
{
	return material
		&& material->iblState().useIbl
		&& environmentTargets != nullptr
		&& environmentTargets->isInitialized()
		&& environmentTargets->hasPrecomputedEnvironment();
}

bool PBRIBLResourceBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<PBRMaterial>& material,
	const EnvironmentRenderTargets* environmentTargets
)
{
	if (!shader || !material)
	{
		return false;
	}

	const bool useIBL = canUseIBL(material, environmentTargets);
	shader->setInt("useIBL", useIBL ? 1 : 0);
	for (const auto& slot : material->getIblFloatUniformSlots())
	{
		shader->setFloat(slot.uniformName, slot.value ? *slot.value : 0.0f);
	}

	if (!useIBL)
	{
		return false;
	}

	const unsigned int maxMipLevels = environmentTargets->getMaxPrefilterMipLevels();
	shader->setFloat("iblMaxReflectionLod", maxMipLevels > 0 ? static_cast<float>(maxMipLevels - 1) : 0.0f);
	bindTexture(shader, "irradianceMap", environmentTargets->getIrradianceMap());
	bindTexture(shader, "prefilterMap", environmentTargets->getPrefilterMap());
	bindTexture(shader, "brdfLut", environmentTargets->getBrdfLut());
	return true;
}
