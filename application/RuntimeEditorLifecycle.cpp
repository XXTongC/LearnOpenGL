#include "RuntimeEditorLifecycle.h"

#include "RuntimeEditorLifecycleConfig.h"
#include "RuntimeEditorLifecycleState.h"
#include "RuntimeFrameCallbacks.h"
#include "RuntimeEditorPanelCoordinator.h"
#include "RuntimeGuiHost.h"
#include "RuntimeGuiHostTypes.h"

namespace GL_RUNTIME
{
	void RuntimeEditorLifecycle::initialize(
		RuntimeEditorLifecycleState& state,
		const RuntimeEditorLifecycleConfig& config
	)
	{
		state.configureEditorUiModules(config.editorUiModulePolicy);

		if (!config.enableGui)
		{
			return;
		}

		RuntimeGuiHost::initialize({ config.window });
	}

	RuntimeFrameCallbacks RuntimeEditorLifecycle::makeFrameCallbacks(
		GLframework::AppRuntimeContext& context,
		RuntimeEditorLifecycleState& state,
		const RuntimeEditorLifecycleConfig& config
	)
	{
		RuntimeFrameCallbacks callbacks{};
		if (!config.enableGui)
		{
			return callbacks;
		}

		callbacks.renderUi = [&context, &state, config]()
		{
			RuntimeGuiHost::renderFrame({
				config.window,
				[&context, &state, config]()
				{
					RuntimeEditorPanelCoordinator::drawPanels(
						context,
						state,
						config.editorOrbitAngle
					);
					state.applyPendingEditorUiModuleReconfiguration();
				}
			});
		};
		return callbacks;
	}
}
