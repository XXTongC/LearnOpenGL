#include "EnvironmentRenderTargets.h"

#include "framework/texture.h"

using namespace GLframework;

namespace
{
	std::shared_ptr<Texture> createHdrCubemap(unsigned int size, unsigned int unit, bool generateMipmaps)
	{
		auto texture = std::make_shared<Texture>();
		unsigned int textureId{ 0 };

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
		for (unsigned int face = 0; face < 6; ++face)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(
			GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_MIN_FILTER,
			generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR
		);
		if (generateMipmaps)
		{
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		}

		texture->setTexture(textureId);
		texture->setWidth(size);
		texture->setHeight(size);
		texture->setUnit(unit);
		texture->setTextureTarget(GL_TEXTURE_CUBE_MAP);
		texture->setLayerCount(6);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		return texture;
	}

	std::shared_ptr<Texture> createFloatTexture2D(unsigned int width, unsigned int height, unsigned int unit)
	{
		auto texture = std::make_shared<Texture>();
		unsigned int textureId{ 0 };

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		texture->setTexture(textureId);
		texture->setWidth(width);
		texture->setHeight(height);
		texture->setUnit(unit);
		texture->setTextureTarget(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return texture;
	}
}

EnvironmentRenderTargets::~EnvironmentRenderTargets()
{
	releaseCaptureResources();
}

void EnvironmentRenderTargets::initialize(const EnvironmentRenderTargetSettings& settings)
{
	releaseCaptureResources();

	mSettings = settings;
	mHasPrecomputedEnvironment = false;
	mEnvironmentMap = createHdrCubemap(mSettings.environmentSize, mSettings.environmentUnit, false);
	mIrradianceMap = createHdrCubemap(mSettings.irradianceSize, mSettings.irradianceUnit, false);
	mPrefilterMap = createHdrCubemap(mSettings.prefilterSize, mSettings.prefilterUnit, true);
	mBrdfLut = createFloatTexture2D(mSettings.brdfLutSize, mSettings.brdfLutSize, mSettings.brdfLutUnit);

	glGenFramebuffers(1, &mCaptureFbo);
	glGenRenderbuffers(1, &mCaptureRbo);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mSettings.captureDepthSize, mSettings.captureDepthSize);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mCaptureRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool EnvironmentRenderTargets::isInitialized() const
{
	return mCaptureFbo != 0
		&& mCaptureRbo != 0
		&& mEnvironmentMap != nullptr
		&& mIrradianceMap != nullptr
		&& mPrefilterMap != nullptr
		&& mBrdfLut != nullptr;
}

const EnvironmentRenderTargetSettings& EnvironmentRenderTargets::getSettings() const
{
	return mSettings;
}

bool EnvironmentRenderTargets::hasPrecomputedEnvironment() const
{
	return mHasPrecomputedEnvironment;
}

void EnvironmentRenderTargets::setPrecomputedEnvironment(bool value)
{
	mHasPrecomputedEnvironment = value;
}

unsigned int EnvironmentRenderTargets::getCaptureFbo() const
{
	return mCaptureFbo;
}

unsigned int EnvironmentRenderTargets::getCaptureRbo() const
{
	return mCaptureRbo;
}

unsigned int EnvironmentRenderTargets::getMaxPrefilterMipLevels() const
{
	return mSettings.maxPrefilterMipLevels;
}

const std::shared_ptr<Texture>& EnvironmentRenderTargets::getEnvironmentMap() const
{
	return mEnvironmentMap;
}

const std::shared_ptr<Texture>& EnvironmentRenderTargets::getIrradianceMap() const
{
	return mIrradianceMap;
}

const std::shared_ptr<Texture>& EnvironmentRenderTargets::getPrefilterMap() const
{
	return mPrefilterMap;
}

const std::shared_ptr<Texture>& EnvironmentRenderTargets::getBrdfLut() const
{
	return mBrdfLut;
}

void EnvironmentRenderTargets::attachCubemapFaceForCapture(
	const std::shared_ptr<Texture>& cubemap,
	unsigned int faceIndex,
	unsigned int mipLevel,
	unsigned int captureSize
) const
{
	if (!cubemap || faceIndex >= 6)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, captureSize, captureSize);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex,
		cubemap->getTexture(),
		mipLevel
	);
}

void EnvironmentRenderTargets::attachBrdfLutForCapture() const
{
	if (!mBrdfLut)
	{
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mSettings.brdfLutSize, mSettings.brdfLutSize);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBrdfLut->getTexture(), 0);
}

void EnvironmentRenderTargets::releaseCaptureResources()
{
	if (mCaptureRbo != 0)
	{
		glDeleteRenderbuffers(1, &mCaptureRbo);
		mCaptureRbo = 0;
	}

	if (mCaptureFbo != 0)
	{
		glDeleteFramebuffers(1, &mCaptureFbo);
		mCaptureFbo = 0;
	}
}
