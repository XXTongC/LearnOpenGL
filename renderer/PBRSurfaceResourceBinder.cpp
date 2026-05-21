#include "PBRSurfaceResourceBinder.h"

using namespace GLframework;

namespace
{
	void bindOptionalTexture(
		const std::shared_ptr<Shader>& shader,
		const char* samplerName,
		const char* useFlagName,
		const std::shared_ptr<Texture>& texture
	)
	{
		shader->setInt(useFlagName, texture != nullptr ? 1 : 0);
		if (!texture)
		{
			return;
		}

		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}
}

bool PBRSurfaceResourceBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<PBRMaterial>& material
)
{
	if (!shader || !material)
	{
		return false;
	}

	for (const auto& slot : material->getVec3UniformSlots())
	{
		shader->setVector3(slot.uniformName, slot.value ? *slot.value : glm::vec3{ 0.0f });
	}

	for (const auto& slot : material->getSurfaceFloatUniformSlots())
	{
		shader->setFloat(slot.uniformName, slot.value ? *slot.value : 0.0f);
	}
	shader->setInt("useAlphaMask", material->mUseAlphaMask ? 1 : 0);
	shader->setFloat("alphaCutoff", material->mAlphaCutoff);

	for (const auto& slot : material->getTextureSlots())
	{
		bindOptionalTexture(
			shader,
			slot.samplerUniform,
			slot.useFlagUniform,
			slot.texture ? *slot.texture : nullptr
		);
	}

	return true;
}
