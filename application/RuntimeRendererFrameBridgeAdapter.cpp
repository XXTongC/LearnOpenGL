#include "RuntimeRendererFrameBridgeAdapter.h"

#include <cctype>
#include <string>

#include "AppRuntimeContext.h"
#include "RuntimeFrameReadinessResourceAdapter.h"
#include "RuntimeFramePassRegistry.h"
#include "RuntimeFramePipeline.h"
#include "RuntimeFramePipelineProfile.h"
#include "../engine/RendererBackendFrameTypes.h"

namespace
{
	std::string sanitizeProfileToken(const std::string& value)
	{
		std::string token{};
		token.reserve(value.size());
		for (const char ch : value)
		{
			const auto unsignedCh = static_cast<unsigned char>(ch);
			if (std::isalnum(unsignedCh) != 0 || ch == '-' || ch == '_' || ch == ':' || ch == '=')
			{
				token.push_back(ch);
				continue;
			}

			if (ch == ',' || ch == ';' || ch == '|' || std::isspace(unsignedCh) != 0)
			{
				token.push_back('+');
				continue;
			}

			token.push_back('_');
		}

		return token.empty() ? "default" : token;
	}

	char enabledToken(bool enabled)
	{
		return enabled ? '1' : '0';
	}

	std::string makeFramePlanKey(const GL_RUNTIME::RuntimeFramePipelineProfile& profile)
	{
		std::string key = "runtime-frame-pipeline:order=" + sanitizeProfileToken(profile.passOrder);
		key += ":enabled=";
		key.push_back(enabledToken(profile.sceneColorPassEnabled));
		key.push_back(enabledToken(profile.sceneResolvePassEnabled));
		key.push_back(enabledToken(profile.bloomPassEnabled));
		key.push_back(enabledToken(profile.screenCompositePassEnabled));
		return key;
	}

}

namespace GL_RUNTIME
{
	RuntimeRendererFrameBridgeAdapter::RuntimeRendererFrameBridgeAdapter(GLframework::AppRuntimeContext& context)
		: mContext(context)
	{
	}

	const char* RuntimeRendererFrameBridgeAdapter::getBackendKey() const
	{
		return "runtime-frame-pipeline-adapter";
	}

	bool RuntimeRendererFrameBridgeAdapter::isBackendReady() const
	{
		const auto& profile = mContext.profiles.framePipelineProfile();
		const auto passPlan = RuntimeFramePassRegistry::buildPassPlan(profile);
		for (const auto* pass : passPlan)
		{
			if (!pass || !pass->shouldExecute(profile))
			{
				continue;
			}

			if (!RuntimeFrameReadinessResourceAdapter::isFramePassReady(*pass, mContext))
			{
				return false;
			}
		}

		return true;
	}

	GLengine::RendererFrameResult RuntimeRendererFrameBridgeAdapter::renderFrame(
		const GLengine::EngineContext& engineContext,
		const GLengine::RendererFrameIntent& frameIntent
	)
	{
		(void)engineContext;
		return renderRuntimeFrame(mContext, frameIntent);
	}

	GLengine::RendererFrameResult RuntimeRendererFrameBridgeAdapter::renderRuntimeFrame(
		GLframework::AppRuntimeContext& context,
		const GLengine::RendererFrameIntent& frameIntent
	)
	{
		const auto pipelineStats = RuntimeFramePipeline::render(context, {
			frameIntent.framebufferWidth,
			frameIntent.framebufferHeight
		});

		return {
			makeFramePlanKey(context.profiles.framePipelineProfile()),
			pipelineStats.plannedPassCount,
			pipelineStats.executedPassCount,
			pipelineStats.skippedPassCount
		};
	}
}
