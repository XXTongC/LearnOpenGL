#include "RuntimeFramePassRegistry.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>

#include "RuntimeFramePasses.h"
#include "RuntimeFramePipelineProfile.h"

namespace
{
	std::string trim(std::string_view value)
	{
		std::string normalized{ value };
		auto isSpace = [](unsigned char ch)
		{
			return std::isspace(ch) != 0;
		};

		normalized.erase(normalized.begin(), std::find_if(normalized.begin(), normalized.end(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}));
		normalized.erase(std::find_if(normalized.rbegin(), normalized.rend(), [isSpace](char ch)
		{
			return !isSpace(static_cast<unsigned char>(ch));
		}).base(), normalized.end());
		return normalized;
	}

	bool isSceneColorPassEnabled(const GL_RUNTIME::RuntimeFramePipelineProfile& profile)
	{
		return profile.sceneColorPassEnabled;
	}

	bool isSceneResolvePassEnabled(const GL_RUNTIME::RuntimeFramePipelineProfile& profile)
	{
		return profile.sceneResolvePassEnabled;
	}

	bool isBloomPassEnabled(const GL_RUNTIME::RuntimeFramePipelineProfile& profile)
	{
		return profile.bloomPassEnabled;
	}

	bool isScreenCompositePassEnabled(const GL_RUNTIME::RuntimeFramePipelineProfile& profile)
	{
		return profile.screenCompositePassEnabled;
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
	bool RuntimeFramePassDefinition::shouldExecute(const RuntimeFramePipelineProfile& profile) const
	{
		return !isEnabled || isEnabled(profile);
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

	const RuntimeFramePassDefinition* RuntimeFramePassRegistry::findPassByKey(std::string_view key)
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
