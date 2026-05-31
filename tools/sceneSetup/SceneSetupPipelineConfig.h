#pragma once

namespace GL_SCENE
{
	struct SceneSetupPipelineConfig
	{
		bool useLegacyDefaultScene{ true };
		bool useWorldDrivenMinimalScene{ false };
		bool addWorldDrivenProbe{ false };
		bool mirrorLegacySceneToEngineWorld{ true };
	};
}
