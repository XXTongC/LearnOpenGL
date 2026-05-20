#include "PBRDeferredLightingPass.h"

#include <algorithm>
#include <string>

#include "camera/camera.h"
#include "core.h"
#include "framework/geometry.h"
#include "light/ambientLight.h"
#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRGBufferRenderTargets.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

namespace
{
	constexpr int kMaxDeferredPointLights = 4;

	void bindTexture2D(unsigned int texture, unsigned int unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void bindTextureOrDefault(
		const std::shared_ptr<Texture>& texture,
		unsigned int fallbackTarget,
		unsigned int unit
	)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(texture ? texture->getTextureTarget() : fallbackTarget, texture ? texture->getTexture() : 0);
	}
}

int PBRDeferredLightingPass::render(
	const PBRGBufferRenderTargets& targets,
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary
)
{
	if (!targets.isComplete() || context.camera == nullptr)
	{
		return 0;
	}

	const auto shader = shaderLibrary.getPbrDeferredLightingShader();
	if (!shader)
	{
		return 0;
	}

	ensureLightingQuad(shader);
	if (!mLightingQuad)
	{
		return 0;
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	shader->begin();
	bindGBufferTextures(shader, targets);
	bindFrameUniforms(shader, context, profile);

	const bool drawn = MeshDraw::drawIndexed(mLightingQuad);
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	return drawn ? 1 : 0;
}

void PBRDeferredLightingPass::bindFrameUniforms(
	const std::shared_ptr<Shader>& shader,
	const MaterialBindingContext& context,
	const RendererFramePassProfile& profile
) const
{
	shader->setVector3("cameraPosition", context.camera->mPosition);
	shader->setFloat("pbrDeferredLightingIntensity", profile.pbrDeferredLightingIntensity);

	if (context.ambient)
	{
		shader->setVector3("ambientColor", context.ambient->getColor());
	}
	else
	{
		shader->setVector3("ambientColor", glm::vec3{ 0.0f });
	}

	if (context.dirLight)
	{
		shader->setVector3("directionalLight.color", context.dirLight->getColor());
		shader->setVector3("directionalLight.direction", context.dirLight->getDirection());
		shader->setFloat("directionalLight.intensity", context.dirLight->getIntensity());
	}
	else
	{
		shader->setVector3("directionalLight.color", glm::vec3{ 0.0f });
		shader->setVector3("directionalLight.direction", glm::vec3{ 0.0f, -1.0f, 0.0f });
		shader->setFloat("directionalLight.intensity", 0.0f);
	}

	static const std::vector<std::shared_ptr<PointLight>> emptyPointLights{};
	const auto& pointLights = context.pointLights ? *context.pointLights : emptyPointLights;
	const int pointLightCount = std::min(static_cast<int>(pointLights.size()), kMaxDeferredPointLights);
	shader->setInt("POINT_LIGHT_NUM", pointLightCount);
	for (int index = 0; index < pointLightCount; ++index)
	{
		const auto& pointLight = pointLights[static_cast<std::size_t>(index)];
		if (!pointLight)
		{
			continue;
		}

		const std::string baseName = "pointLights[" + std::to_string(index) + "]";
		shader->setVector3(baseName + ".color", pointLight->getColor());
		shader->setVector3(baseName + ".position", pointLight->getPosition());
		shader->setFloat(baseName + ".k2", pointLight->getK2());
		shader->setFloat(baseName + ".k1", pointLight->getK1());
		shader->setFloat(baseName + ".k0", pointLight->getK0());
	}

	const bool useIBL = context.environmentTargets
		&& context.environmentTargets->isInitialized()
		&& context.environmentTargets->hasPrecomputedEnvironment();
	shader->setInt("useIBL", useIBL ? 1 : 0);
	shader->setFloat("iblDiffuseStrength", profile.pbrDeferredIblDiffuseStrength);
	shader->setFloat("iblSpecularStrength", profile.pbrDeferredIblSpecularStrength);
	if (!useIBL)
	{
		return;
	}

	const unsigned int maxMipLevels = context.environmentTargets->getMaxPrefilterMipLevels();
	shader->setFloat("iblMaxReflectionLod", maxMipLevels > 0 ? static_cast<float>(maxMipLevels - 1) : 0.0f);
	shader->setInt("irradianceMap", 4);
	shader->setInt("prefilterMap", 5);
	shader->setInt("brdfLut", 6);
	bindTextureOrDefault(context.environmentTargets->getIrradianceMap(), GL_TEXTURE_CUBE_MAP, 4);
	bindTextureOrDefault(context.environmentTargets->getPrefilterMap(), GL_TEXTURE_CUBE_MAP, 5);
	bindTextureOrDefault(context.environmentTargets->getBrdfLut(), GL_TEXTURE_2D, 6);
}

void PBRDeferredLightingPass::bindGBufferTextures(
	const std::shared_ptr<Shader>& shader,
	const PBRGBufferRenderTargets& targets
) const
{
	shader->setInt("positionRoughnessTexture", 0);
	shader->setInt("normalMetallicTexture", 1);
	shader->setInt("albedoAoTexture", 2);
	shader->setInt("depthTexture", 3);

	bindTexture2D(targets.getPositionRoughnessTexture(), 0);
	bindTexture2D(targets.getNormalMetallicTexture(), 1);
	bindTexture2D(targets.getAlbedoAoTexture(), 2);
	bindTexture2D(targets.getDepthTexture(), 3);
}

void PBRDeferredLightingPass::ensureLightingQuad(const std::shared_ptr<Shader>& shader)
{
	if (mLightingQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mLightingQuad = std::make_shared<Mesh>(geometry, nullptr);
	mLightingQuad->setName("PBR Deferred Lighting Quad");
}
