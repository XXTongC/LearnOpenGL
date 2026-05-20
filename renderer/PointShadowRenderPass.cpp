#include "PointShadowRenderPass.h"

#include "camera/perspectivecamera.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "renderer/ShadowMeshDraw.h"
#include "tools/tools.h"

using namespace GLframework;

ShadowRenderStats PointShadowRenderPass::render(
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	ShaderLibrary& shaderLibrary
) const
{
	ShadowRenderStats stats{};
	if (ShadowMeshDraw::isPostProcessPass(meshes) || pointLights.empty())
	{
		return stats;
	}

	GLint preFbo{ 0 };
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &preFbo);

	GLint preViewPort[4]{};
	glGetIntegerv(GL_VIEWPORT, preViewPort);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	auto depthTexture = PointLightShadow::getSharedDepthTexture();
	const int width = depthTexture->getWidth();
	const int height = depthTexture->getHeight();

	GLuint tempFBO{ 0 };
	glGenFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
	glViewport(0, 0, width, height);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	stats.pointLightCount = static_cast<int>(pointLights.size());
	for (size_t i = 0; i < pointLights.size(); ++i)
	{
		const auto& pointLight = pointLights[i];
		if (!pointLight || !pointLight->getShadow())
		{
			continue;
		}

		const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
		pointShadow->setShadowMapIndex(static_cast<int>(i));
		auto shadowDistanceShader = shaderLibrary.getShadowDistanceShader();

		for (unsigned int face = 0; face < 6; ++face)
		{
			++stats.pointFaceCount;
			const int layerIndex = pointShadow->getShadowMapIndex() * 6 + static_cast<int>(face);
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture->getTexture(), 0, layerIndex);
			glClear(GL_DEPTH_BUFFER_BIT);

			const glm::mat4 shadowProj = std::static_pointer_cast<PerspectiveCamera>(pointShadow->mCamera)->getProjectionMatrix();
			const glm::mat4 shadowView = glm::lookAt(
				pointLight->getPosition(),
				pointLight->getPosition() + Tools::getCubemapFaceDirection(face),
				Tools::getCubemapFaceUp(face)
			);

			shadowDistanceShader->begin();
			shadowDistanceShader->setMat4("lightSpaceMatrix", shadowProj * shadowView);
			shadowDistanceShader->setVector3("lightPos", pointLight->getPosition());
			shadowDistanceShader->setFloat("far_plane", pointShadow->mCamera->mFar);

			for (const auto& mesh : meshes)
			{
				shadowDistanceShader->setMat4("modelMatrix", mesh->getModelMatrix());
				ShadowMeshDraw::draw(mesh);
				++stats.pointDrawCalls;
			}

			shadowDistanceShader->end();
		}
	}

	glDeleteFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
	return stats;
}
