#include "PBRGBufferDebugPass.h"

#include <algorithm>

#include "core.h"
#include "framework/geometry.h"
#include "renderer/MeshDraw.h"
#include "renderer/PBRGBufferRenderTargets.h"
#include "renderer/RendererFramePassProfile.h"
#include "renderer/ShaderLibrary.h"

using namespace GLframework;

namespace
{
	void bindTexture(unsigned int texture, unsigned int unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
}

int PBRGBufferDebugPass::render(
	const PBRGBufferRenderTargets& targets,
	const RendererFramePassProfile& profile,
	ShaderLibrary& shaderLibrary
)
{
	if (!targets.isComplete())
	{
		return 0;
	}

	const auto shader = shaderLibrary.getPbrGBufferDebugShader();
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
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_POLYGON_OFFSET_LINE);

	shader->begin();
	shader->setInt("positionRoughnessTexture", 0);
	shader->setInt("normalMetallicTexture", 1);
	shader->setInt("albedoAoTexture", 2);
	shader->setInt("emissiveTexture", 4);
	shader->setInt("materialParamsTexture", 5);
	shader->setInt("depthTexture", 3);
	shader->setInt("gbufferDebugMode", std::clamp(profile.pbrGBufferDebugMode, 0, 8));
	shader->setFloat("gbufferDebugIntensity", profile.pbrGBufferDebugIntensity);

	bindTexture(targets.getPositionRoughnessTexture(), 0);
	bindTexture(targets.getNormalMetallicTexture(), 1);
	bindTexture(targets.getAlbedoAoTexture(), 2);
	bindTexture(targets.getDepthTexture(), 3);
	bindTexture(targets.getEmissiveTexture(), 4);
	bindTexture(targets.getMaterialParamsTexture(), 5);

	const bool drawn = MeshDraw::drawIndexed(mDebugQuad);
	shader->end();

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	return drawn ? 1 : 0;
}

void PBRGBufferDebugPass::ensureDebugQuad(const std::shared_ptr<Shader>& shader)
{
	if (mDebugQuad || !shader)
	{
		return;
	}

	auto geometry = Geometry::createScreenPlane(shader);
	mDebugQuad = std::make_shared<Mesh>(geometry, nullptr);
	mDebugQuad->setName("PBR GBuffer Debug Quad");
}
