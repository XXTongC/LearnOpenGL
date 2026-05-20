#include "PBRShadowAtlasRenderPass.h"

#include <algorithm>

#include "camera/perspectivecamera.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "renderer/ShadowMeshDraw.h"
#include "tools/tools.h"

using namespace GLframework;

PBRShadowAtlasStats PBRShadowAtlasRenderPass::render(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	PBRShadowAtlasRenderTargets& targets,
	ShaderLibrary& shaderLibrary
) const
{
	PBRShadowAtlasStats stats = targets.prepare(dirLight, pointLights);
	if (ShadowMeshDraw::isPostProcessPass(meshes) || !stats.ready)
	{
		return stats;
	}

	GLint previousFbo{ 0 };
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFbo);

	GLint previousViewport[4]{};
	glGetIntegerv(GL_VIEWPORT, previousViewport);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glBindFramebuffer(GL_FRAMEBUFFER, targets.getFramebuffer());
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	stats.directionalDrawCalls = renderDirectionalAtlas(
		camera,
		meshes,
		dirLight,
		targets,
		stats,
		shaderLibrary
	);
	stats.pointDrawCalls = renderPointAtlas(
		meshes,
		pointLights,
		targets,
		stats,
		shaderLibrary,
		stats.pointFacesRendered
	);

	glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
	glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
	return stats;
}

int PBRShadowAtlasRenderPass::renderDirectionalAtlas(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const PBRShadowAtlasRenderTargets& targets,
	const PBRShadowAtlasStats& atlasStats,
	ShaderLibrary& shaderLibrary
) const
{
	if (camera == nullptr || !dirLight || !dirLight->getShadow() || targets.getDirectionalDepthTexture() == 0)
	{
		return 0;
	}

	const auto csmShadow = std::dynamic_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());
	if (!csmShadow)
	{
		return 0;
	}

	std::vector<float> layers;
	csmShadow->generateCascadeLayers(layers, camera->mNear, camera->mFar);
	const auto lightMatrices = csmShadow->getLightMatrix(camera, dirLight->getDirection(), layers);
	const int layerCount = std::min(
		atlasStats.directionalLayerCount,
		static_cast<int>(lightMatrices.size())
	);

	auto shadowShader = shaderLibrary.getShadowShader();
	if (!shadowShader)
	{
		return 0;
	}

	int drawCalls = 0;
	glViewport(0, 0, atlasStats.directionalResolution, atlasStats.directionalResolution);
	for (int layer = 0; layer < layerCount; ++layer)
	{
		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			targets.getDirectionalDepthTexture(),
			0,
			layer
		);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadowShader->begin();
		shadowShader->setMat4("lightMatrix", lightMatrices[static_cast<std::size_t>(layer)]);
		for (const auto& mesh : meshes)
		{
			shadowShader->setMat4("modelMatrix", mesh->getModelMatrix());
			if (ShadowMeshDraw::draw(mesh))
			{
				++drawCalls;
			}
		}
		shadowShader->end();
	}

	return drawCalls;
}

int PBRShadowAtlasRenderPass::renderPointAtlas(
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const PBRShadowAtlasRenderTargets& targets,
	const PBRShadowAtlasStats& atlasStats,
	ShaderLibrary& shaderLibrary,
	int& renderedFaceCount
) const
{
	renderedFaceCount = 0;
	if (pointLights.empty() || targets.getPointDepthTexture() == 0)
	{
		return 0;
	}

	auto shadowDistanceShader = shaderLibrary.getShadowDistanceShader();
	if (!shadowDistanceShader)
	{
		return 0;
	}

	int drawCalls = 0;
	int atlasPointIndex = 0;
	glViewport(0, 0, atlasStats.pointResolution, atlasStats.pointResolution);
	for (const auto& pointLight : pointLights)
	{
		if (!pointLight || !pointLight->getShadow() || atlasPointIndex >= PBRShadowAtlasRenderTargets::maxPointLights())
		{
			continue;
		}

		const auto pointShadow = std::dynamic_pointer_cast<PointLightShadow>(pointLight->getShadow());
		if (!pointShadow)
		{
			continue;
		}

		for (unsigned int face = 0; face < 6; ++face)
		{
			const int layerIndex = atlasPointIndex * 6 + static_cast<int>(face);
			glFramebufferTextureLayer(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				targets.getPointDepthTexture(),
				0,
				layerIndex
			);
			glClear(GL_DEPTH_BUFFER_BIT);
			++renderedFaceCount;

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
				if (ShadowMeshDraw::draw(mesh))
				{
					++drawCalls;
				}
			}
			shadowDistanceShader->end();
		}

		++atlasPointIndex;
	}

	return drawCalls;
}
