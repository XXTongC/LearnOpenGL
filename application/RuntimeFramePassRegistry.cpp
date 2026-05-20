#include "RuntimeFramePassRegistry.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

#include "AppRuntimeContext.h"
#include "RuntimeFramePasses.h"
#include "RuntimeFramePipeline.h"
#include "RuntimeFramePipelineProfile.h"

namespace
{
	std::string trim(std::string value)
	{
		auto isSpace = [](unsigned char ch)
		{
			return std::isspace(ch) != 0;
		};

		value.erase(value.begin(), std::find_if(value.begin(), value.end(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}));
		value.erase(std::find_if(value.rbegin(), value.rend(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}).base(), value.end());
		return value;
	}

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

	bool containsPassId(
		const std::vector<const GL_RUNTIME::RuntimeFramePassDefinition*>& passes,
		GL_RUNTIME::RuntimeFramePassId id
	)
	{
		return std::any_of(passes.begin(), passes.end(), [id](const GL_RUNTIME::RuntimeFramePassDefinition* pass)
		{
			return pass && pass->id == id;
		});
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
				"SceneColor",
				"Scene Color",
				isSceneColorPassEnabled,
				executeSceneColorPass
			},
			{
				RuntimeFramePassId::SceneResolve,
				"SceneResolve",
				"Scene Resolve",
				isSceneResolvePassEnabled,
				executeSceneResolvePass
			},
			{
				RuntimeFramePassId::Bloom,
				"Bloom",
				"Bloom",
				isBloomPassEnabled,
				executeBloomPass
			},
			{
				RuntimeFramePassId::ScreenComposite,
				"ScreenComposite",
				"Screen Composite",
				isScreenCompositePassEnabled,
				executeScreenCompositePass
			}
		};

		return passes;
	}

	const RuntimeFramePassDefinition* RuntimeFramePassRegistry::findPassByKey(const std::string& key)
	{
		const auto normalizedKey = trim(key);
		if (normalizedKey.empty())
		{
			return nullptr;
		}

		for (const auto& pass : defaultPasses())
		{
			if (normalizedKey == pass.key || normalizedKey == pass.debugName)
			{
				return &pass;
			}
		}

		return nullptr;
	}

	std::vector<const RuntimeFramePassDefinition*> RuntimeFramePassRegistry::buildPassPlan(
		const RuntimeFramePipelineProfile& profile
	)
	{
		std::vector<const RuntimeFramePassDefinition*> plan{};
		std::stringstream stream{ profile.passOrder };
		std::string token{};
		while (std::getline(stream, token, ','))
		{
			const auto* pass = findPassByKey(token);
			if (pass && !containsPassId(plan, pass->id))
			{
				plan.push_back(pass);
			}
		}

		if (!plan.empty())
		{
			return plan;
		}

		for (const auto& pass : defaultPasses())
		{
			plan.push_back(&pass);
		}
		return plan;
	}
}
