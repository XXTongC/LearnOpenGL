#pragma once

namespace GLframework
{
	struct PostProcessSettings;
}

namespace GL_RUNTIME
{
	struct RuntimeCameraLightState;
	struct RuntimeRenderResourceState;

	class RuntimeFrameRenderResourceAdapter
	{
	public:
		static void renderSceneColor(
			RuntimeRenderResourceState& renderResources,
			const RuntimeCameraLightState& cameraLights
		);
		static void resolveSceneColor(RuntimeRenderResourceState& renderResources);
		static void renderBloom(
			RuntimeRenderResourceState& renderResources,
			const GLframework::PostProcessSettings& postProcessSettings
		);
		static void renderScreenComposite(
			RuntimeRenderResourceState& renderResources,
			const GLframework::PostProcessSettings& postProcessSettings,
			int framebufferWidth,
			int framebufferHeight
		);
	};
}
