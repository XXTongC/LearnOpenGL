#include "RuntimePBRPreviewProfileVerification.h"

#include "AppRuntimeContext.h"
#include "RuntimeVerificationConfig.h"

namespace GL_RUNTIME
{
	void RuntimePBRPreviewProfileVerification::applyPreviewProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& probes = verification.pbr.probes;
		const auto& engineWorld = verification.engineWorld;

		context.profiles.pbrPreviewProfile.enabled = true;
		context.profiles.pbrPreviewProfile.position = { 0.0f, 0.0f, 1.2f };
		context.profiles.pbrPreviewProfile.segments = 32;
		context.profiles.pbrPreviewProfile.rings = 16;
		context.profiles.pbrPreviewProfile.useMaterialGrid = true;
		context.profiles.pbrPreviewProfile.gridColumns = 5;
		context.profiles.pbrPreviewProfile.gridRows = 5;
		context.profiles.pbrPreviewProfile.gridSpacing = 0.9f;
		context.profiles.pbrPreviewProfile.gridRadius = 0.32f;
		context.profiles.pbrPreviewProfile.gridMetallicMin = 0.0f;
		context.profiles.pbrPreviewProfile.gridMetallicMax = 1.0f;
		context.profiles.pbrPreviewProfile.gridRoughnessMin = 0.08f;
		context.profiles.pbrPreviewProfile.gridRoughnessMax = 1.0f;
		context.profiles.pbrPreviewProfile.material.albedo = { 0.9f, 0.42f, 0.18f };
		context.profiles.pbrPreviewProfile.material.ao = 1.0f;
		context.profiles.pbrPreviewProfile.material.useIBL = true;
		context.profiles.pbrPreviewProfile.material.iblDiffuseStrength = 1.0f;
		context.profiles.pbrPreviewProfile.material.iblSpecularStrength = 1.0f;
		context.profiles.pbrPreviewProfile.normalMapPath = "Texture/normal/normal_map.png";
		context.profiles.pbrPreviewProfile.normalMapUnit = 4;
		if (probes.enablePbrShowcaseSpheres)
		{
			context.profiles.pbrPreviewProfile.position = { -0.15f, -0.95f, 1.25f };
			context.profiles.pbrPreviewProfile.gridColumns = 5;
			context.profiles.pbrPreviewProfile.gridRows = 4;
			context.profiles.pbrPreviewProfile.gridSpacing = 0.72f;
			context.profiles.pbrPreviewProfile.gridRadius = 0.24f;
			context.profiles.pbrPreviewProfile.gridRoughnessMin = 0.06f;
			context.profiles.pbrPreviewProfile.gridRoughnessMax = 0.92f;
			context.profiles.pbrPreviewProfile.material.albedo = { 0.82f, 0.38f, 0.16f };
		}
		if (engineWorld.enableMinimalScene)
		{
			context.profiles.pbrPreviewProfile.enabled = false;
		}
	}
}
