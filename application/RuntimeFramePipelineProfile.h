#pragma once

#include <string>

namespace GL_EDITOR
{
	class PropertyBuilder;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineProfile
	{
		bool sceneColorPassEnabled{ true };
		bool sceneResolvePassEnabled{ true };
		bool bloomPassEnabled{ true };
		bool screenCompositePassEnabled{ true };

		void visitEditableProperties(GL_EDITOR::PropertyBuilder& builder);
	};

	class RuntimeFramePipelineProfileStorage
	{
	public:
		static std::string defaultPath();
		static bool loadFromFile(const std::string& path, RuntimeFramePipelineProfile& profile);
		static bool saveToFile(const std::string& path, const RuntimeFramePipelineProfile& profile);
	};
}
