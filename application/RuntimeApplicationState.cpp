#include "RuntimeApplicationState.h"

#include <memory>

#include "AppRuntimeContext.h"
#include "RuntimeEditorLifecycleState.h"
#include "RuntimeEngineLifecycleState.h"
#include "RuntimeFrameLifecycleState.h"

#include "../engine/Engine.h"
#include "../tools/legacyExperiments/LegacyExperimentRunner.h"

namespace GL_RUNTIME
{
	RuntimeApplicationState::RuntimeApplicationState()
		: mEngine(std::make_unique<GLengine::Engine>())
		, mEngineLifecycle(std::make_unique<RuntimeEngineLifecycleState>())
		, mRuntime(std::make_unique<GLframework::AppRuntimeContext>())
		, mEditorLifecycle(std::make_unique<RuntimeEditorLifecycleState>())
		, mLegacyExperiments(std::make_unique<GL_EXPERIMENTS::LegacyExperimentRunner>())
		, mFrameLifecycle(std::make_unique<RuntimeFrameLifecycleState>())
	{
	}

	RuntimeApplicationState::~RuntimeApplicationState() = default;

	GLengine::Engine& RuntimeApplicationState::engine()
	{
		return *mEngine;
	}

	const GLengine::Engine& RuntimeApplicationState::engine() const
	{
		return *mEngine;
	}

	RuntimeEngineLifecycleState& RuntimeApplicationState::engineLifecycle()
	{
		return *mEngineLifecycle;
	}

	const RuntimeEngineLifecycleState& RuntimeApplicationState::engineLifecycle() const
	{
		return *mEngineLifecycle;
	}

	GLframework::AppRuntimeContext& RuntimeApplicationState::runtime()
	{
		return *mRuntime;
	}

	const GLframework::AppRuntimeContext& RuntimeApplicationState::runtime() const
	{
		return *mRuntime;
	}

	RuntimeEditorLifecycleState& RuntimeApplicationState::editorLifecycle()
	{
		return *mEditorLifecycle;
	}

	const RuntimeEditorLifecycleState& RuntimeApplicationState::editorLifecycle() const
	{
		return *mEditorLifecycle;
	}

	GL_EXPERIMENTS::LegacyExperimentRunner& RuntimeApplicationState::legacyExperiments()
	{
		return *mLegacyExperiments;
	}

	const GL_EXPERIMENTS::LegacyExperimentRunner& RuntimeApplicationState::legacyExperiments() const
	{
		return *mLegacyExperiments;
	}

	RuntimeFrameLifecycleState& RuntimeApplicationState::frameLifecycle()
	{
		return *mFrameLifecycle;
	}

	const RuntimeFrameLifecycleState& RuntimeApplicationState::frameLifecycle() const
	{
		return *mFrameLifecycle;
	}
}
