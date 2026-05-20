#pragma once

namespace GLframework
{
	class PBRGBufferRenderTargets
	{
	public:
		PBRGBufferRenderTargets() = default;
		~PBRGBufferRenderTargets();
		PBRGBufferRenderTargets(const PBRGBufferRenderTargets&) = delete;
		PBRGBufferRenderTargets& operator=(const PBRGBufferRenderTargets&) = delete;

		bool ensureSize(unsigned int width, unsigned int height);
		void release();

		bool isComplete() const;
		unsigned int getFbo() const;
		unsigned int getWidth() const;
		unsigned int getHeight() const;
		unsigned int getPositionRoughnessTexture() const;
		unsigned int getNormalMetallicTexture() const;
		unsigned int getAlbedoAoTexture() const;
		unsigned int getEmissiveTexture() const;
		unsigned int getMaterialParamsTexture() const;
		unsigned int getDepthTexture() const;

	private:
		unsigned int createColorAttachment(unsigned int width, unsigned int height) const;
		unsigned int createDepthAttachment(unsigned int width, unsigned int height) const;

		unsigned int mFbo{ 0 };
		unsigned int mPositionRoughnessTexture{ 0 };
		unsigned int mNormalMetallicTexture{ 0 };
		unsigned int mAlbedoAoTexture{ 0 };
		unsigned int mEmissiveTexture{ 0 };
		unsigned int mMaterialParamsTexture{ 0 };
		unsigned int mDepthTexture{ 0 };
		unsigned int mWidth{ 0 };
		unsigned int mHeight{ 0 };
		bool mComplete{ false };
	};
}
