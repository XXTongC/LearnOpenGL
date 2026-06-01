#include "RuntimeFramePipelineProfile.h"

#include "RuntimeFramePipelineProfileConfig.h"
#include "../tools/config/ProfileConfigIO.h"

namespace GL_RUNTIME
{
	std::string RuntimeFramePipelineProfileStorage::defaultPath()
	{
		return "config/runtime_frame_pipeline.local.ini";
	}

	bool RuntimeFramePipelineProfileStorage::loadFromFile(
		const std::string& path,
		RuntimeFramePipelineProfile& profile
	)
	{
		RuntimeFramePipelineProfile loadedProfile = profile;
		GL_EDITOR::PropertyBuilder builder{};
		buildRuntimeFramePipelineProfileConfigSchema(builder, loadedProfile);
		const bool loaded = GL_CONFIG::loadPropertyConfig(path, builder);
		if (!loaded)
		{
			return false;
		}

		profile = loadedProfile;
		return true;
	}

	bool RuntimeFramePipelineProfileStorage::saveToFile(
		const std::string& path,
		const RuntimeFramePipelineProfile& profile
	)
	{
		RuntimeFramePipelineProfile snapshot = profile;
		GL_EDITOR::PropertyBuilder builder{};
		buildRuntimeFramePipelineProfileConfigSchema(builder, snapshot);
		return GL_CONFIG::savePropertyConfig(
			path,
			"# Local runtime frame pipeline pass plan and toggles",
			builder
		);
	}
}
