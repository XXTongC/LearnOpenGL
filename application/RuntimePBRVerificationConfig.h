#pragma once

namespace GL_RUNTIME
{
	struct RuntimePBRVerificationPassConfig
	{
		bool enableIblDebugPass{ false };
		bool enablePbrGBufferPass{ false };
		bool enablePbrDeferredLightingPass{ false };
		bool enablePbrDeferredTiledLightDebugPass{ false };
		bool enablePbrDeferredClusteredLightDebugPass{ false };
		bool enablePbrGBufferDebugPass{ false };
		bool disablePbrShadowAtlasPass{ false };
	};

	struct RuntimePBRVerificationProbeConfig
	{
		bool enablePbrTransparentFallbackPass{ false };
		bool enablePbrEmissiveProbe{ false };
		bool enablePbrMaterialIblProbe{ false };
		bool enablePbrAlphaMaskProbe{ false };
		bool enablePbrImportedAssetProbe{ false };
		bool enablePbrTextureSetProbe{ false };
		bool enablePbrShowcaseSpheres{ false };
		bool enablePbrTiledLightProbe{ false };
		bool enablePbrClusteredLayoutProbe{ false };
		bool enablePbrClusteredGridProbe{ false };
		bool enablePbrClusteredStatsReadback{ false };
		bool enablePbrLightPressureProbe{ false };
	};

	struct RuntimePBRVerificationDeferredConfig
	{
		bool disablePbrDeferredTiledLights{ false };
		int pbrDeferredTileSizeOverride{ 0 };
		float pbrDeferredTiledLightCutoffOverride{ 0.0f };
	};

	struct RuntimePBRVerificationConfig
	{
		RuntimePBRVerificationPassConfig passes{};
		RuntimePBRVerificationProbeConfig probes{};
		RuntimePBRVerificationDeferredConfig deferred{};
	};

}
