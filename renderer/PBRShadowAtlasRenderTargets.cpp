#include "PBRShadowAtlasRenderTargets.h"

#include <algorithm>

#include "light/directionalLight.h"
#include "light/pointLight.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"

using namespace GLframework;

namespace
{
	constexpr int kDirectionalAtlasResolution = 1024;
	constexpr int kPointAtlasResolution = 512;

	int getDirectionalLayerCount(const std::shared_ptr<DirectionalLight>& dirLight)
	{
		if (!dirLight || !dirLight->getShadow())
		{
			return 0;
		}

		const auto csmShadow = std::dynamic_pointer_cast<DirectionalLightCSMShadow>(dirLight->getShadow());
		return csmShadow ? csmShadow->getLayerCount() : 0;
	}

	int getPointLightCount(const std::vector<std::shared_ptr<PointLight>>& pointLights)
	{
		int count = 0;
		for (const auto& pointLight : pointLights)
		{
			if (pointLight && pointLight->getShadow())
			{
				++count;
			}
		}
		return count;
	}
}

PBRShadowAtlasRenderTargets::~PBRShadowAtlasRenderTargets()
{
	releaseTexture(mDirectionalDepthTexture, mDirectionalLayerCapacity);
	releaseTexture(mPointDepthTexture, mPointFaceLayerCapacity);
	if (mFramebuffer != 0)
	{
		glDeleteFramebuffers(1, &mFramebuffer);
		mFramebuffer = 0;
	}
}

PBRShadowAtlasStats PBRShadowAtlasRenderTargets::prepare(
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights
)
{
	const int directionalLayerCount = std::clamp(
		getDirectionalLayerCount(dirLight),
		0,
		maxDirectionalLayers()
	);
	const int pointLightCount = std::clamp(
		getPointLightCount(pointLights),
		0,
		maxPointLights()
	);

	ensureFramebuffer();
	ensureDirectionalAtlas(directionalLayerCount);
	ensurePointAtlas(pointLightCount);

	mLastStats = PBRShadowAtlasStats{
		mFramebuffer != 0 && (mDirectionalDepthTexture != 0 || mPointDepthTexture != 0),
		mDirectionalDepthTexture != 0,
		mPointDepthTexture != 0,
		directionalLayerCount,
		pointLightCount,
		pointLightCount * 6,
		0,
		0,
		0,
		kDirectionalAtlasResolution,
		kPointAtlasResolution,
		mFramebuffer,
		mDirectionalDepthTexture,
		mPointDepthTexture
	};
	return mLastStats;
}

const PBRShadowAtlasStats& PBRShadowAtlasRenderTargets::getLastStats() const
{
	return mLastStats;
}

unsigned int PBRShadowAtlasRenderTargets::getFramebuffer() const
{
	return mFramebuffer;
}

unsigned int PBRShadowAtlasRenderTargets::getDirectionalDepthTexture() const
{
	return mDirectionalDepthTexture;
}

unsigned int PBRShadowAtlasRenderTargets::getPointDepthTexture() const
{
	return mPointDepthTexture;
}

void PBRShadowAtlasRenderTargets::ensureFramebuffer()
{
	if (mFramebuffer != 0)
	{
		return;
	}

	glGenFramebuffers(1, &mFramebuffer);
}

void PBRShadowAtlasRenderTargets::ensureDirectionalAtlas(int layerCount)
{
	if (layerCount <= 0)
	{
		releaseTexture(mDirectionalDepthTexture, mDirectionalLayerCapacity);
		return;
	}

	if (mDirectionalDepthTexture != 0 && mDirectionalLayerCapacity == layerCount)
	{
		return;
	}

	releaseTexture(mDirectionalDepthTexture, mDirectionalLayerCapacity);
	mDirectionalDepthTexture = createDepthArrayTexture(kDirectionalAtlasResolution, layerCount);
	mDirectionalLayerCapacity = mDirectionalDepthTexture != 0 ? layerCount : 0;
}

void PBRShadowAtlasRenderTargets::ensurePointAtlas(int pointLightCount)
{
	const int faceLayerCount = pointLightCount * 6;
	if (faceLayerCount <= 0)
	{
		releaseTexture(mPointDepthTexture, mPointFaceLayerCapacity);
		return;
	}

	if (mPointDepthTexture != 0 && mPointFaceLayerCapacity == faceLayerCount)
	{
		return;
	}

	releaseTexture(mPointDepthTexture, mPointFaceLayerCapacity);
	mPointDepthTexture = createDepthArrayTexture(kPointAtlasResolution, faceLayerCount);
	mPointFaceLayerCapacity = mPointDepthTexture != 0 ? faceLayerCount : 0;
}

void PBRShadowAtlasRenderTargets::releaseTexture(unsigned int& texture, int& layerCapacity)
{
	if (texture != 0)
	{
		glDeleteTextures(1, &texture);
		texture = 0;
	}
	layerCapacity = 0;
}

unsigned int PBRShadowAtlasRenderTargets::createDepthArrayTexture(int resolution, int layers) const
{
	if (resolution <= 0 || layers <= 0)
	{
		return 0;
	}

	GLint previousTexture{ 0 };
	glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &previousTexture);

	unsigned int texture{ 0 };
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(
		GL_TEXTURE_2D_ARRAY,
		0,
		GL_DEPTH_COMPONENT32F,
		resolution,
		resolution,
		layers,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr
	);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<GLuint>(previousTexture));
	return texture;
}
