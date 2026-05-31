#include "RuntimeEngineLifecycle.h"

#include "AppRuntimeContext.h"
#include "RuntimeEngineLifecycleCleanupRefs.h"
#include "RuntimeEngineLifecycleState.h"
#include "RuntimeRendererBackendAttachmentLifecycle.h"
#include "../engine/AssetSubsystem.h"
#include "../engine/Engine.h"
#include "../engine/RendererSubsystem.h"

namespace GL_RUNTIME
{
	bool RuntimeEngineLifecycle::initializeEngine(
		GLframework::AppRuntimeContext& context,
		GLengine::Engine& engine,
		RuntimeEngineLifecycleState& state,
		const GLengine::EngineDesc& engineDesc
	)
	{
		context.engineAttachments.engine = &engine;
		if (!state.assetSubsystem)
		{
			state.assetSubsystem = &engine.addSubsystem<GLengine::AssetSubsystem>();
		}
		context.engineAttachments.assetSubsystem = state.assetSubsystem;

		if (!state.rendererSubsystem)
		{
			state.rendererSubsystem = &engine.addSubsystem<GLengine::RendererSubsystem>();
		}
		context.engineAttachments.rendererSubsystem = state.rendererSubsystem;

		return engine.initialize(engineDesc);
	}

	bool RuntimeEngineLifecycle::attachRendererBackend(
		GLframework::AppRuntimeContext& context,
		RuntimeEngineLifecycleState& state,
		std::string_view rendererBackendKey
	)
	{
		if (!state.rendererSubsystem)
		{
			return false;
		}

		return RuntimeRendererBackendAttachmentLifecycle::attachToRendererSubsystem(
			context,
			*state.rendererSubsystem,
			rendererBackendKey
		);
	}

	RuntimeEngineLifecycleCleanupRefs RuntimeEngineLifecycle::beginCleanup(
		RuntimeEngineLifecycleState& state
	)
	{
		RuntimeEngineLifecycleCleanupRefs refs{
			state.assetSubsystem,
			state.rendererSubsystem
		};
		if (state.rendererSubsystem)
		{
			state.rendererSubsystem->clearRendererBackend();
		}
		return refs;
	}

	void RuntimeEngineLifecycle::detachRuntimeContext(
		GLframework::AppRuntimeContext& context,
		RuntimeEngineLifecycleState& state
	)
	{
		context.engineAttachments.engine = nullptr;
		context.engineAttachments.engineWorld = nullptr;
		context.engineAttachments.engineWorldEditable = false;
		context.engineAttachments.assetSubsystem = nullptr;
		context.engineAttachments.rendererSubsystem = nullptr;
		state.assetSubsystem = nullptr;
		state.rendererSubsystem = nullptr;
	}

	void RuntimeEngineLifecycle::shutdownEngine(GLengine::Engine& engine)
	{
		engine.shutdown();
	}
}
