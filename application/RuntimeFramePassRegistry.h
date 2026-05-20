#pragma once

#include <string>
#include <vector>

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GL_RUNTIME
{
	struct RuntimeFramePipelineConfig;
	struct RuntimeFramePipelineProfile;

	enum class RuntimeFramePassId
	{
		SceneColor,
		SceneResolve,
		Bloom,
		ScreenComposite
	};

	using RuntimeFramePassEnabledPredicate = bool (*)(const GLframework::AppRuntimeContext& context);
	using RuntimeFramePassExecutor = void (*)(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	);

	struct RuntimeFramePassDefinition
	{
		RuntimeFramePassId id{ RuntimeFramePassId::SceneColor };
		const char* key{ "" };
		const char* debugName{ "" };
		RuntimeFramePassEnabledPredicate isEnabled{ nullptr };
		RuntimeFramePassExecutor execute{ nullptr };

		bool shouldExecute(const GLframework::AppRuntimeContext& context) const;
		void executePass(
			GLframework::AppRuntimeContext& context,
			const RuntimeFramePipelineConfig& config
		) const;
	};

	class RuntimeFramePassRegistry
	{
	public:
		static const std::vector<RuntimeFramePassDefinition>& defaultPasses();
		static const RuntimeFramePassDefinition* findPassByKey(const std::string& key);
		static std::vector<const RuntimeFramePassDefinition*> buildPassPlan(
			const RuntimeFramePipelineProfile& profile
		);
	};
}
