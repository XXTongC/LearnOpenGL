#include "ShadowRenderer.h"

#include "camera/perspectivecamera.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "materials/material.h"
#include "mesh/instancedMesh.h"
#include "tools/tools.h"

using namespace GLframework;

void ShadowRenderer::render(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	ShaderLibrary& shaderLibrary
)
{
	renderDirectionalShadowMap(camera, meshes, dirLight, shaderLibrary);
	renderPointShadowMap(meshes, pointLights, shaderLibrary);
}

void ShadowRenderer::renderDirectionalShadowMap(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	ShaderLibrary& shaderLibrary
)
{
	if (isPostProcessPass(meshes))
	{
		return;
	}

	GLint preFbo{ 0 };
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &preFbo);

	GLint preViewPort[4]{};
	glGetIntegerv(GL_VIEWPORT, preViewPort);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	auto csmShadow = std::static_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());
	glBindFramebuffer(GL_FRAMEBUFFER, csmShadow->mRenderTarget->getFBO());

	std::vector<float> layers;
	csmShadow->generateCascadeLayers(layers, camera->mNear, camera->mFar);
	auto lightMatrices = csmShadow->getLightMatrix(camera, dirLight->getDirection(), layers);
	glViewport(0, 0, csmShadow->mRenderTarget->getWidth(), csmShadow->mRenderTarget->getHeight());

	for (int i = 0; i < csmShadow->getLayerCount(); ++i)
	{
		auto shadowShader = shaderLibrary.getShadowShader();
		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			csmShadow->mRenderTarget->getDepthAttachment()->getTexture(),
			0,
			i
		);
		glClear(GL_DEPTH_BUFFER_BIT);

		shadowShader->begin();
		shadowShader->setMat4("lightMatrix", lightMatrices[i]);
		for (const auto& mesh : meshes)
		{
			shadowShader->setMat4("modelMatrix", mesh->getModelMatrix());
			drawShadowMesh(mesh);
		}
		shadowShader->end();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
}

void ShadowRenderer::renderPointShadowMap(
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	ShaderLibrary& shaderLibrary
)
{
	if (isPostProcessPass(meshes))
	{
		return;
	}
	if (pointLights.empty())
	{
		return;
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

	for (size_t i = 0; i < pointLights.size(); ++i)
	{
		const auto& pointLight = pointLights[i];
		const auto& pointShadow = std::static_pointer_cast<PointLightShadow>(pointLight->getShadow());
		pointShadow->setShadowMapIndex(static_cast<int>(i));
		auto shadowDistanceShader = shaderLibrary.getShadowDistanceShader();

		for (unsigned int face = 0; face < 6; ++face)
		{
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
				drawShadowMesh(mesh);
			}

			shadowDistanceShader->end();
		}
	}

	glDeleteFramebuffers(1, &tempFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
}

bool ShadowRenderer::isPostProcessPass(const std::vector<std::shared_ptr<Mesh>>& meshes) const
{
	for (const auto& mesh : meshes)
	{
		if (mesh->getMaterial()->getMaterialType() != MaterialType::ScreenMaterial)
		{
			return false;
		}
	}

	return true;
}

void ShadowRenderer::drawShadowMesh(const std::shared_ptr<Mesh>& mesh) const
{
	auto geometry = mesh->getGeometry();
	glBindVertexArray(geometry->getVao());

	if (mesh->getType() == ObjectType::InstancedMesh)
	{
		const auto im = std::static_pointer_cast<InstancedMesh>(mesh);
		glDrawElementsInstanced(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr, im->getInstanceCount());
	}
	else
	{
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);
}
