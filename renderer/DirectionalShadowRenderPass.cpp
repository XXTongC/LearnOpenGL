#include "DirectionalShadowRenderPass.h"

#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "renderer/PBRAlphaShadowBinder.h"
#include "renderer/ShadowMeshDraw.h"

using namespace GLframework;

ShadowRenderStats DirectionalShadowRenderPass::render(
	Camera* camera,
	const std::vector<std::shared_ptr<Mesh>>& meshes,
	const std::shared_ptr<DirectionalLight>& dirLight,
	ShaderLibrary& shaderLibrary
) const
{
	ShadowRenderStats stats{};
	if (ShadowMeshDraw::isPostProcessPass(meshes) || camera == nullptr || !dirLight || !dirLight->getShadow())
	{
		return stats;
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

	stats.directionalLayerCount = csmShadow->getLayerCount();
	for (int i = 0; i < csmShadow->getLayerCount(); ++i)
	{
		auto shadowShader = shaderLibrary.getShadowShader();
		auto alphaShadowShader = shaderLibrary.getPbrAlphaShadowShader();
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
			if (PBRAlphaShadowBinder::isAlphaMaskedPbrMesh(mesh))
			{
				continue;
			}

			shadowShader->setMat4("modelMatrix", mesh->getModelMatrix());
			if (ShadowMeshDraw::draw(mesh))
			{
				++stats.directionalDrawCalls;
			}
		}
		shadowShader->end();

		if (alphaShadowShader)
		{
			alphaShadowShader->begin();
			for (const auto& mesh : meshes)
			{
				if (!PBRAlphaShadowBinder::bindDirectional(alphaShadowShader, mesh, lightMatrices[i]))
				{
					continue;
				}

				if (ShadowMeshDraw::draw(mesh))
				{
					++stats.directionalDrawCalls;
					++stats.directionalAlphaMaskedDrawCalls;
				}
			}
			alphaShadowShader->end();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, preFbo);
	glViewport(preViewPort[0], preViewPort[1], preViewPort[2], preViewPort[3]);
	return stats;
}
