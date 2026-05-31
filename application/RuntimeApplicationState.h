#pragma once

#include <memory>

namespace GLframework
{
	struct AppRuntimeContext;
}

namespace GLengine
{
	class Engine;
}

namespace GL_EXPERIMENTS
{
	class LegacyExperimentRunner;
}

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleState;
	struct RuntimeEngineLifecycleState;
	struct RuntimeFrameLifecycleState;

	struct RuntimeApplicationState
	{
	private:
		std::unique_ptr<GLengine::Engine> mEngine{};

	public:
		RuntimeApplicationState();
		~RuntimeApplicationState();

		RuntimeApplicationState(const RuntimeApplicationState&) = delete;
		RuntimeApplicationState& operator=(const RuntimeApplicationState&) = delete;

		GLengine::Engine& engine();
		const GLengine::Engine& engine() const;

	private:
		std::unique_ptr<RuntimeEngineLifecycleState> mEngineLifecycle{};

	public:
		RuntimeEngineLifecycleState& engineLifecycle();
		const RuntimeEngineLifecycleState& engineLifecycle() const;

	private:
		std::unique_ptr<GLframework::AppRuntimeContext> mRuntime{};

	public:
		GLframework::AppRuntimeContext& runtime();
		const GLframework::AppRuntimeContext& runtime() const;

	private:
		std::unique_ptr<RuntimeEditorLifecycleState> mEditorLifecycle{};

	public:
		RuntimeEditorLifecycleState& editorLifecycle();
		const RuntimeEditorLifecycleState& editorLifecycle() const;

	private:
		std::unique_ptr<GL_EXPERIMENTS::LegacyExperimentRunner> mLegacyExperiments{};

	public:
		GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments();
		const GL_EXPERIMENTS::LegacyExperimentRunner& legacyExperiments() const;

	private:
		std::unique_ptr<RuntimeFrameLifecycleState> mFrameLifecycle{};

	public:
		RuntimeFrameLifecycleState& frameLifecycle();
		const RuntimeFrameLifecycleState& frameLifecycle() const;
	};
}
