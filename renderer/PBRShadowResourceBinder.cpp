#include "PBRShadowResourceBinder.h"

#include <array>

#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "light/shadow/pointLightShadow/pointLightShadow.h"
#include "renderer/PBRShadowAtlasRenderTargets.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	constexpr int pbrCsmShadowTextureUnit = 8;
	constexpr int pbrPointShadowTextureUnit = 9;

	struct PBRPointShadowAtlasUniforms
	{
		int shaderLightCount{ 0 };
		int shadowLightCount{ 0 };
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> enabled{};
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> layerBase{};
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> nearPlane{};
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> farPlane{};
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> bias{};
		std::array<float, PBRShadowAtlasRenderTargets::maxPointLights()> pcfRadius{};
	};

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

	bool canBindPbrDirectionalShadowAtlas(const MaterialBindingContext& context)
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

	bool canBindPbrPointShadowAtlas(const MaterialBindingContext& context)
	{
		if (!context.pbrShadowAtlasTargets)
		{
			return false;
		}

		const auto& atlasStats = context.pbrShadowAtlasTargets->getLastStats();
		return atlasStats.ready
			&& atlasStats.pointReady
			&& atlasStats.pointLightCount > 0
			&& context.pbrShadowAtlasTargets->getPointDepthTexture() != 0;
	}

	void bindDepthTextureArray(unsigned int texture, int textureUnit)
	{
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	}

	void bindPointShadowAtlasDefaults(const std::shared_ptr<Shader>& shader)
	{
		if (!shader)
		{
			return;
		}

		shader->setInt("pbrPointShadowAtlasEnabled", 0);
		shader->setInt("pbrPointShadowAtlasLightCount", 0);
	}

	PBRPointShadowAtlasUniforms buildPointShadowAtlasUniforms(const MaterialBindingContext& context)
	{
		PBRPointShadowAtlasUniforms uniforms{};
		int atlasPointIndex = 0;
		for (const auto& pointLight : context.getPointLights())
		{
			if (!pointLight || uniforms.shaderLightCount >= PBRShadowAtlasRenderTargets::maxPointLights())
			{
				continue;
			}

			const int shaderLightIndex = uniforms.shaderLightCount;
			++uniforms.shaderLightCount;

			const auto pointShadow = std::dynamic_pointer_cast<PointLightShadow>(pointLight->getShadow());
			if (!pointShadow || !pointShadow->mCamera || atlasPointIndex >= PBRShadowAtlasRenderTargets::maxPointLights())
			{
				continue;
			}

			uniforms.enabled[static_cast<std::size_t>(shaderLightIndex)] = 1.0f;
			uniforms.layerBase[static_cast<std::size_t>(shaderLightIndex)] = static_cast<float>(atlasPointIndex * 6);
			uniforms.nearPlane[static_cast<std::size_t>(shaderLightIndex)] = pointShadow->mCamera->mNear;
			uniforms.farPlane[static_cast<std::size_t>(shaderLightIndex)] = pointShadow->mCamera->mFar;
			uniforms.bias[static_cast<std::size_t>(shaderLightIndex)] = pointShadow->mBias;
			uniforms.pcfRadius[static_cast<std::size_t>(shaderLightIndex)] = pointShadow->mPcfRadius;
			++uniforms.shadowLightCount;
			++atlasPointIndex;
		}
		return uniforms;
	}

	bool bindPbrPointShadowAtlas(
		const std::shared_ptr<Shader>& shader,
		const MaterialBindingContext& context,
		PBRShadowResourceBindResult& result
	)
	{
		bindPointShadowAtlasDefaults(shader);
		if (!shader || !canBindPbrPointShadowAtlas(context))
		{
			return false;
		}

		PBRPointShadowAtlasUniforms uniforms = buildPointShadowAtlasUniforms(context);
		if (uniforms.shaderLightCount <= 0 || uniforms.shadowLightCount <= 0)
		{
			return false;
		}

		shader->setInt("pbrPointShadowMapSampler", pbrPointShadowTextureUnit);
		shader->setInt("pbrPointShadowAtlasEnabled", 1);
		shader->setInt("pbrPointShadowAtlasLightCount", uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowEnabled", uniforms.enabled.data(), uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowLayerBase", uniforms.layerBase.data(), uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowNear", uniforms.nearPlane.data(), uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowFar", uniforms.farPlane.data(), uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowBias", uniforms.bias.data(), uniforms.shaderLightCount);
		shader->setFloatArray("pbrPointShadowPcfRadius", uniforms.pcfRadius.data(), uniforms.shaderLightCount);
		bindDepthTextureArray(
			context.pbrShadowAtlasTargets->getPointDepthTexture(),
			pbrPointShadowTextureUnit
		);

		result.pointShadowAtlasBound = true;
		result.pointShadowAtlasLightCount = uniforms.shadowLightCount;
		return true;
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

	PBRShadowResourceBindResult result{};
	bindPointShadowAtlasDefaults(shader);

	const auto csmShadow = getCsmShadow(context);
	if (!csmShadow || csmShadow->getLayerCount() <= 0)
	{
		shader->setInt("csmLayerCount", 0);
		bindPbrPointShadowAtlas(shader, context, result);
		return result;
	}

	if (canBindPbrDirectionalShadowAtlas(context))
	{
		if (!bindCsmFrameUniforms(shader, context, csmShadow))
		{
			bindPbrPointShadowAtlas(shader, context, result);
			return result;
		}

		shader->setInt("shadowMapSampler", pbrCsmShadowTextureUnit);
		bindDepthTextureArray(
			context.pbrShadowAtlasTargets->getDirectionalDepthTexture(),
			pbrCsmShadowTextureUnit
		);
		result.bound = true;
		result.csmLayerCount = csmShadow->getLayerCount();
		result.source = PBRShadowResourceSource::PBRShadowAtlas;
		bindPbrPointShadowAtlas(shader, context, result);
		return result;
	}

	ShadowResourceBinder::bindCSMShadowResources(
		shader,
		context.camera,
		context.dirLight,
		pbrCsmShadowTextureUnit
	);
	result.bound = true;
	result.csmLayerCount = csmShadow->getLayerCount();
	result.source = PBRShadowResourceSource::LegacyCSM;
	bindPbrPointShadowAtlas(shader, context, result);
	return result;
}

int PBRShadowResourceBinder::getCsmLayerCount(const MaterialBindingContext& context)
{
	const auto dirCSMShadow = getCsmShadow(context);
	return dirCSMShadow ? dirCSMShadow->getLayerCount() : 0;
}
