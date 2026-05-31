#pragma once

namespace GL_SCENE
{
	struct SceneSetupPipelineResult;
}

namespace GL_RUNTIME
{
	class RuntimeSceneSetupReport
	{
	public:
		static void reportPreparedScene(const GL_SCENE::SceneSetupPipelineResult& result);
		static void reportRendererPrepared();
	};
}
