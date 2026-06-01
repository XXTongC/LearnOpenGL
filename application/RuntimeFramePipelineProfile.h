#pragma once

#include <string>

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile
	{
		std::string passOrder{ "SceneColor,SceneResolve,Bloom,ScreenComposite" };
		bool sceneColorPassEnabled{ true };
		bool sceneResolvePassEnabled{ true };
		bool bloomPassEnabled{ true };
		bool screenCompositePassEnabled{ true };
	};

	class RuntimeFramePipelineProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, RuntimeFramePipelineProfile& profile);
		static bool saveToFile(const std::string& path, const RuntimeFramePipelineProfile& profile);
	};
}
