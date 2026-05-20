#include "PBRShadowResourceBinder.h"

#include "light/directionalLight.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "renderer/PBRShadowAtlasRenderTargets.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	constexpr int pbrCsmShadowTextureUnit = 8;

	std::shared_ptr<DirectionalLightCSMShadow> getCsmShadow(const MaterialBindingContext& context)
	{
		if (context.camera == nullptr || !context.dirLight || !context.dirLight->getShadow())
		{
			return nullptr;
		}

		return std::dynamic_pointer_cast<DirectionalLightCSMShadow>(context.dirLight->getShadow());
	}

	bool bindCsmFrameUniforms(
		const std::shared_ptr<Shader>& shader,
		const MaterialBindingContext& context,
		const std::shared_ptr<DirectionalLightCSMShadow>& csmShadow
	)
	{
		if (!shader || !csmShadow || context.camera == nullptr || !context.dirLight)
		{
			return false;
		}

		std::vector<float> layers;
		csmShadow->generateCascadeLayers(layers, context.camera->mNear, context.camera->mFar);
		const auto lightMatrices = csmShadow->getLightMatrix(context.camera, context.dirLight->getDirection(), layers);
		if (layers.empty() || lightMatrices.empty())
		{
			shader->setInt("csmLayerCount", 0);
			return false;
		}

		shader->setInt("csmLayerCount", csmShadow->getLayerCount());
		shader->setFloatArray("csmLayers", layers.data(), static_cast<int>(layers.size()));
		shader->setMat4Array("lightMatrices", lightMatrices.data(), static_cast<int>(lightMatrices.size()));
		shader->setFloat("lightSize", csmShadow->mLightSize);
		shader->setMat4("lightViewMatrix", glm::inverse(context.dirLight->getModelMatrix()));
		shader->setFloat("bias", csmShadow->mBias);
		shader->setFloat("diskTightness", csmShadow->mDiskTightness);
		shader->setFloat("pcfRadius", csmShadow->mPcfRadius);
		return true;
	}

	bool canBindPbrShadowAtlas(const MaterialBindingContext& context)
	{
		if (!context.pbrShadowAtlasTargets)
		{
			return false;
		}

		const auto& atlasStats = context.pbrShadowAtlasTargets->getLastStats();
		return atlasStats.ready
			&& atlasStats.directionalReady
			&& context.pbrShadowAtlasTargets->getDirectionalDepthTexture() != 0;
	}

	void bindDepthTextureArray(unsigned int texture, int textureUnit)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	}
}

bool PBRShadowResourceBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const MaterialBindingContext& context
)
{
	return bindDetailed(shader, context).bound;
}

PBRShadowResourceBindResult PBRShadowResourceBinder::bindDetailed(
	const std::shared_ptr<Shader>& shader,
	const MaterialBindingContext& context
)
{
	if (!shader)
	{
		return {};
	}

	const auto csmShadow = getCsmShadow(context);
	if (!csmShadow || csmShadow->getLayerCount() <= 0)
	{
		shader->setInt("csmLayerCount", 0);
		return {};
	}

	if (canBindPbrShadowAtlas(context))
	{
		if (!bindCsmFrameUniforms(shader, context, csmShadow))
		{
			return {};
		}

		shader->setInt("shadowMapSampler", pbrCsmShadowTextureUnit);
		bindDepthTextureArray(
			context.pbrShadowAtlasTargets->getDirectionalDepthTexture(),
			pbrCsmShadowTextureUnit
		);
		return PBRShadowResourceBindResult{
			true,
			csmShadow->getLayerCount(),
			PBRShadowResourceSource::PBRShadowAtlas
		};
	}

	ShadowResourceBinder::bindCSMShadowResources(
		shader,
		context.camera,
		context.dirLight,
		pbrCsmShadowTextureUnit
	);
	return PBRShadowResourceBindResult{
		true,
		csmShadow->getLayerCount(),
		PBRShadowResourceSource::LegacyCSM
	};
}

int PBRShadowResourceBinder::getCsmLayerCount(const MaterialBindingContext& context)
{
	const auto dirCSMShadow = getCsmShadow(context);
	return dirCSMShadow ? dirCSMShadow->getLayerCount() : 0;
}
