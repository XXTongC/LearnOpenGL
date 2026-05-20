#include "RuntimeFramePassRegistry.h"

#include "AppRuntimeContext.h"
#include "RuntimeFramePasses.h"
#include "RuntimeFramePipeline.h"

namespace
{
	bool isSceneColorPassEnabled(const GLframework::AppRuntimeContext& context)
	{
		return context.framePipelineProfile.sceneColorPassEnabled;
	}

	bool isSceneResolvePassEnabled(const GLframework::AppRuntimeContext& context)
	{
		return context.framePipelineProfile.sceneResolvePassEnabled;
	}

	bool isBloomPassEnabled(const GLframework::AppRuntimeContext& context)
	{
		return context.framePipelineProfile.bloomPassEnabled;
	}

	bool isScreenCompositePassEnabled(const GLframework::AppRuntimeContext& context)
	{
		return context.framePipelineProfile.screenCompositePassEnabled;
	}

	void executeSceneColorPass(
		GLframework::AppRuntimeContext& context,
		const GL_RUNTIME::RuntimeFramePipelineConfig&
	)
	{
		GL_RUNTIME::RuntimeSceneColorPass::execute(context);
	}

	void executeSceneResolvePass(
		GLframework::AppRuntimeContext& context,
		const GL_RUNTIME::RuntimeFramePipelineConfig&
	)
	{
		GL_RUNTIME::RuntimeSceneResolvePass::execute(context);
	}

	void executeBloomPass(
		GLframework::AppRuntimeContext& context,
		const GL_RUNTIME::RuntimeFramePipelineConfig&
	)
	{
		GL_RUNTIME::RuntimeBloomPass::execute(context);
	}

	void executeScreenCompositePass(
		GLframework::AppRuntimeContext& context,
		const GL_RUNTIME::RuntimeFramePipelineConfig& config
	)
	{
		GL_RUNTIME::RuntimeScreenCompositePass::execute(context, config);
	}
}

namespace GL_RUNTIME
{
	bool RuntimeFramePassDefinition::shouldExecute(const GLframework::AppRuntimeContext& context) const
	{
		return !isEnabled || isEnabled(context);
	}

	void RuntimeFramePassDefinition::executePass(
		GLframework::AppRuntimeContext& context,
		const RuntimeFramePipelineConfig& config
	) const
	{
		if (execute)
		{
			execute(context, config);
		}
	}

	const std::vector<RuntimeFramePassDefinition>& RuntimeFramePassRegistry::defaultPasses()
	{
		static const std::vector<RuntimeFramePassDefinition> passes{
			{
				RuntimeFramePassId::SceneColor,
				"Scene Color",
				isSceneColorPassEnabled,
				executeSceneColorPass
			},
			{
				RuntimeFramePassId::SceneResolve,
				"Scene Resolve",
				isSceneResolvePassEnabled,
				executeSceneResolvePass
			},
			{
				RuntimeFramePassId::Bloom,
				"Bloom",
				isBloomPassEnabled,
				executeBloomPass
			},
			{
				RuntimeFramePassId::ScreenComposite,
				"Screen Composite",
				isScreenCompositePassEnabled,
				executeScreenCompositePass
			}
		};

		return passes;
	}
}
