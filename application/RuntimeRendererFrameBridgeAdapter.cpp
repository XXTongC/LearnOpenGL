#include "RuntimeRendererFrameBridgeAdapter.h"

#include <cctype>
#include <string>

#include "AppRuntimeContext.h"
#include "RuntimeFramePassRegistry.h"
#include "RuntimeFramePipeline.h"
#include "RuntimeFramePipelineProfile.h"

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

	bool isSceneColorReady(const GLframework::AppRuntimeContext& context)
	{
		return context.renderResources.renderer != nullptr
			&& context.renderResources.sceneOffScreen != nullptr
			&& context.cameraLights.camera != nullptr
			&& context.renderResources.frameRenderTargets.isInitialized()
			&& context.renderResources.frameRenderTargets.getSceneFbo() != 0;
	}

	bool isSceneResolveReady(const GLframework::AppRuntimeContext& context)
	{
		return context.renderResources.frameRenderTargets.getMultisample() != nullptr
			&& context.renderResources.frameRenderTargets.getResolved() != nullptr;
	}

	bool isBloomReady(const GLframework::AppRuntimeContext& context)
	{
		if (!context.profiles.postProcessSettings.bloomEnabled)
		{
			return true;
		}

		return context.renderResources.bloom != nullptr
			&& context.renderResources.frameRenderTargets.getResolved() != nullptr
			&& context.renderResources.frameRenderTargets.getBloomBright() != nullptr
			&& context.renderResources.frameRenderTargets.getBloomPing() != nullptr
			&& context.renderResources.frameRenderTargets.getBloomPong() != nullptr;
	}

	bool isScreenCompositeReady(const GLframework::AppRuntimeContext& context)
	{
		return context.renderResources.renderer != nullptr
			&& context.renderResources.screenQuad != nullptr
			&& context.renderResources.frameRenderTargets.isInitialized();
	}

	bool isFramePassReady(
		const GL_RUNTIME::RuntimeFramePassDefinition& pass,
		const GLframework::AppRuntimeContext& context
	)
	{
		switch (pass.id)
		{
		case GL_RUNTIME::RuntimeFramePassId::SceneColor:
			return isSceneColorReady(context);
		case GL_RUNTIME::RuntimeFramePassId::SceneResolve:
			return isSceneResolveReady(context);
		case GL_RUNTIME::RuntimeFramePassId::Bloom:
			return isBloomReady(context);
		case GL_RUNTIME::RuntimeFramePassId::ScreenComposite:
			return isScreenCompositeReady(context);
		default:
			return false;
		}
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
		const auto passPlan = RuntimeFramePassRegistry::buildPassPlan(mContext.profiles.framePipelineProfile);
		for (const auto* pass : passPlan)
		{
			if (!pass || !pass->shouldExecute(mContext))
			{
				continue;
			}

			if (!isFramePassReady(*pass, mContext))
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
			makeFramePlanKey(context.profiles.framePipelineProfile),
			pipelineStats.plannedPassCount,
			pipelineStats.executedPassCount,
			pipelineStats.skippedPassCount
		};
	}
}
