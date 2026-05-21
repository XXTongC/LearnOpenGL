#pragma once

#include <string>

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimePBRVerificationConfig
	{
		bool enabled{ false };
		bool enableIblDebugPass{ false };
		bool enablePbrGBufferPass{ false };
		bool enablePbrDeferredLightingPass{ false };
		bool enablePbrDeferredTiledLightDebugPass{ false };
		bool enablePbrGBufferDebugPass{ false };
		bool disablePbrShadowAtlasPass{ false };
		bool enablePbrTransparentFallbackPass{ false };
		bool enablePbrEmissiveProbe{ false };
		bool enablePbrMaterialIblProbe{ false };
		bool enablePbrAlphaMaskProbe{ false };
		bool enablePbrImportedAssetProbe{ false };
		bool enablePbrTextureSetProbe{ false };
		bool enablePbrTiledLightProbe{ false };
		bool enablePbrClusteredLayoutProbe{ false };
		bool enablePbrClusteredGridProbe{ false };
		bool disablePbrDeferredTiledLights{ false };
		int pbrDeferredTileSizeOverride{ 0 };
		float pbrDeferredTiledLightCutoffOverride{ 0.0f };
		int maxFrames{ 3 };
		int captureFrame{ 2 };
		std::string capturePath{ "out/pbr_verification.ppm" };
	};

	class RuntimePBRVerification
	{
	public:
		static void applyProfile(
			GLframework::AppRuntimeContext& context,
			const RuntimePBRVerificationConfig& config
		);
		static void applyRendererPassProfile(
			GLframework::AppRuntimeContext& context,
			const RuntimePBRVerificationConfig& config
		);
		static void addVerificationSceneProbes(
			GLframework::AppRuntimeContext& context,
			const RuntimePBRVerificationConfig& config
		);
		static void reportPreparedScene(GLframework::AppRuntimeContext& context);
		static void reportRenderedFrame(GLframework::AppRuntimeContext& context);
		static bool captureDefaultFramebuffer(
			const std::string& path,
			unsigned int width,
			unsigned int height
		);
	};
}
