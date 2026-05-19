#pragma once

#include <memory>

#include "framework/texture.h"

namespace GLframework
{
	struct EnvironmentRenderTargetSettings
	{
		unsigned int environmentSize{ 512 };
		unsigned int irradianceSize{ 32 };
		unsigned int prefilterSize{ 128 };
		unsigned int brdfLutSize{ 512 };
		unsigned int captureDepthSize{ 512 };
		unsigned int environmentUnit{ 10 };
		unsigned int irradianceUnit{ 11 };
		unsigned int prefilterUnit{ 12 };
		unsigned int brdfLutUnit{ 13 };
		unsigned int maxPrefilterMipLevels{ 5 };
	};

	class EnvironmentRenderTargets
	{
	public:
		EnvironmentRenderTargets() = default;
		~EnvironmentRenderTargets();
		EnvironmentRenderTargets(const EnvironmentRenderTargets&) = delete;
		EnvironmentRenderTargets& operator=(const EnvironmentRenderTargets&) = delete;

		void initialize(const EnvironmentRenderTargetSettings& settings = {});
		bool isInitialized() const;

		const EnvironmentRenderTargetSettings& getSettings() const;
		unsigned int getCaptureFbo() const;
		unsigned int getCaptureRbo() const;
		unsigned int getMaxPrefilterMipLevels() const;

		const std::shared_ptr<Texture>& getEnvironmentMap() const;
		const std::shared_ptr<Texture>& getIrradianceMap() const;
		const std::shared_ptr<Texture>& getPrefilterMap() const;
		const std::shared_ptr<Texture>& getBrdfLut() const;

		void attachCubemapFaceForCapture(
			const std::shared_ptr<Texture>& cubemap,
			unsigned int faceIndex,
			unsigned int mipLevel,
			unsigned int captureSize
		) const;
		void attachBrdfLutForCapture() const;

	private:
		void releaseCaptureResources();

		EnvironmentRenderTargetSettings mSettings{};
		unsigned int mCaptureFbo{ 0 };
		unsigned int mCaptureRbo{ 0 };
		std::shared_ptr<Texture> mEnvironmentMap{ nullptr };
		std::shared_ptr<Texture> mIrradianceMap{ nullptr };
		std::shared_ptr<Texture> mPrefilterMap{ nullptr };
		std::shared_ptr<Texture> mBrdfLut{ nullptr };
	};
}
