#include "RuntimePBRPreviewProfileVerification.h"

#include "AppRuntimeContext.h"
#include "RuntimeVerificationConfig.h"
#include "../tools/sceneSetup/PBRPreviewProfile.h"

namespace GL_RUNTIME
{
	void RuntimePBRPreviewProfileVerification::applyPreviewProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& probes = verification.pbr.probes;
		const auto& engineWorld = verification.engineWorld;
		auto& pbrPreviewProfile = context.profiles.pbrPreviewProfile();

		pbrPreviewProfile.enabled = true;
		pbrPreviewProfile.position = { 0.0f, 0.0f, 1.2f };
		pbrPreviewProfile.segments = 32;
		pbrPreviewProfile.rings = 16;
		pbrPreviewProfile.useMaterialGrid = true;
		pbrPreviewProfile.gridColumns = 5;
		pbrPreviewProfile.gridRows = 5;
		pbrPreviewProfile.gridSpacing = 0.9f;
		pbrPreviewProfile.gridRadius = 0.32f;
		pbrPreviewProfile.gridMetallicMin = 0.0f;
		pbrPreviewProfile.gridMetallicMax = 1.0f;
		pbrPreviewProfile.gridRoughnessMin = 0.08f;
		pbrPreviewProfile.gridRoughnessMax = 1.0f;
		pbrPreviewProfile.material.albedo = { 0.9f, 0.42f, 0.18f };
		pbrPreviewProfile.material.ao = 1.0f;
		pbrPreviewProfile.material.useIBL = true;
		pbrPreviewProfile.material.iblDiffuseStrength = 1.0f;
		pbrPreviewProfile.material.iblSpecularStrength = 1.0f;
		pbrPreviewProfile.normalMapPath = "Texture/normal/normal_map.png";
		pbrPreviewProfile.normalMapUnit = 4;
		if (probes.enablePbrShowcaseSpheres)
		{
			pbrPreviewProfile.position = { -0.15f, -0.95f, 1.25f };
			pbrPreviewProfile.gridColumns = 5;
			pbrPreviewProfile.gridRows = 4;
			pbrPreviewProfile.gridSpacing = 0.72f;
			pbrPreviewProfile.gridRadius = 0.24f;
			pbrPreviewProfile.gridRoughnessMin = 0.06f;
			pbrPreviewProfile.gridRoughnessMax = 0.92f;
			pbrPreviewProfile.material.albedo = { 0.82f, 0.38f, 0.16f };
		}
		if (engineWorld.enableMinimalScene)
		{
			pbrPreviewProfile.enabled = false;
		}
	}
}
