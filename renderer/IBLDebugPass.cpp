#include "IBLDebugPass.h"

#include <algorithm>

#include "core.h"
#include "framework/geometry.h"
#include "framework/texture.h"
#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/MeshDraw.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

namespace
{
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

int IBLDebugPass::render(
	const EnvironmentRenderTargets& environmentTargets,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary
)
{
	if (!environmentTargets.hasPrecomputedEnvironment())
	{
		return 0;
	}

	const auto shader = shaderLibrary.getIblDebugShader();
	if (!shader)
	{
		return 0;
	}

	ensureDebugQuad(shader);
	if (!mDebugQuad)
	{
		return 0;
	}

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	shader->begin();
	shader->setInt("iblDebugMode", std::clamp(profile.iblDebugMode, 0, 3));
	shader->setFloat("iblDebugMipLevel", profile.iblDebugMipLevel);
	shader->setFloat("iblDebugIntensity", profile.iblDebugIntensity);
	shader->setInt("environmentMap", 0);
	shader->setInt("irradianceMap", 1);
	shader->setInt("prefilterMap", 2);
	shader->setInt("brdfLut", 3);

	bindTextureOrDefault(environmentTargets.getEnvironmentMap(), GL_TEXTURE_CUBE_MAP, 0);
	bindTextureOrDefault(environmentTargets.getIrradianceMap(), GL_TEXTURE_CUBE_MAP, 1);
	bindTextureOrDefault(environmentTargets.getPrefilterMap(), GL_TEXTURE_CUBE_MAP, 2);
	bindTextureOrDefault(environmentTargets.getBrdfLut(), GL_TEXTURE_2D, 3);

	const bool drawn = MeshDraw::drawIndexed(mDebugQuad);
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	return drawn ? 1 : 0;
}

void IBLDebugPass::ensureDebugQuad(const std::shared_ptr<Shader>& shader)
{
	if (mDebugQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mDebugQuad = std::make_shared<Mesh>(geometry, nullptr);
	mDebugQuad->setName("IBL Debug Quad");
}
