#include "RuntimeFramePipelineProfile.h"

#include "../tools/config/ProfileConfigIO.h"
#include "../tools/inspector/PropertySchema.h"

namespace GL_RUNTIME
{
	void RuntimeFramePipelineProfile::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
	{
		builder.addSection("Frame Pipeline Passes");
		builder.addConfigBool("sceneColorPassEnabled", "Scene Color Pass", &sceneColorPassEnabled);
		builder.addConfigBool("sceneResolvePassEnabled", "Scene Resolve Pass", &sceneResolvePassEnabled);
		builder.addConfigBool("bloomPassEnabled", "Bloom Pass", &bloomPassEnabled);
		builder.addConfigBool("screenCompositePassEnabled", "Screen Composite Pass", &screenCompositePassEnabled);
	}

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
		loadedProfile.visitEditableProperties(builder);
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
		snapshot.visitEditableProperties(builder);
		return GL_CONFIG::savePropertyConfig(
			path,
			"# Local runtime frame pipeline pass toggles",
			builder
		);
	}
}
