#pragma once

namespace GLengine
{
	class Engine;
}

namespace GL_EXPERIMENTS
{
	struct RuntimeContext;
}

namespace GL_SCENE
{
	struct SetupContext;
}

namespace GL_RUNTIME
{
	struct RuntimeCameraLightState;
	struct RuntimeEngineAttachmentState;
	struct RuntimeProfileState;
	struct RuntimeRenderResourceState;
	struct RuntimeScenePrepareConfig;

	class RuntimeSceneSetupResourceAdapter
	{
	public:
		static GL_SCENE::SetupContext makeSceneSetupContext(
			RuntimeRenderResourceState& renderResources,
			RuntimeCameraLightState& cameraLights,
			RuntimeProfileState& profiles,
			RuntimeEngineAttachmentState& engineAttachments,
			GLengine::Engine& engine,
			const RuntimeScenePrepareConfig& config
		);

		static GL_EXPERIMENTS::RuntimeContext makeLegacyExperimentContext(
			RuntimeRenderResourceState& renderResources,
			RuntimeCameraLightState& cameraLights
		);
	};
}
