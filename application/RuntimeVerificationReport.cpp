#include "RuntimeVerificationReport.h"

#include <cstddef>
#include <iostream>
#include <string>

#include "../engine/AssetRegistry.h"
#include "../engine/AssetSubsystem.h"
#include "../engine/Engine.h"
#include "../engine/EngineLifecycleSnapshot.h"
#include "../engine/RendererSubsystem.h"
#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"
#include "RuntimeEngineVerificationReport.h"
#include "RuntimeRendererBackendResourceAdapter.h"
#include "RuntimeRendererBackendVerificationReport.h"

namespace
{
	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}
}

namespace GL_RUNTIME
{
	void RuntimeVerificationReport::reportRenderedFrameRuntimeStats(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine,
		const GLengine::RendererSubsystem* rendererSubsystem
	)
	{
		const auto engineSnapshot = engine.captureLifecycleSnapshot();
		const auto* activeWorld = engine.getActiveWorld();
		const bool runtimeContextEngineAttached = context.engineAttachments.engine == &engine;
		const bool contextWorldMatchesActive = context.engineAttachments.engineWorld != nullptr
			&& context.engineAttachments.engineWorld == activeWorld;
		reportLine(formatRuntimeEngineLifecycleSnapshotStats(engineSnapshot));
		reportLine(formatRuntimeEngineSubsystemSummaryStats(engineSnapshot));
		reportLine(formatRuntimeEngineTickStats(engineSnapshot, runtimeContextEngineAttached));
		reportLine(formatRuntimeSubsystemHealthStats(engineSnapshot, contextWorldMatchesActive));

		if (!rendererSubsystem)
		{
			return;
		}

		const auto& rendererSubsystemStats = rendererSubsystem->getFrameBridgeStats();
		const bool runtimeRendererAttached =
			RuntimeRendererBackendResourceAdapter::isRuntimeRendererAttached(
				*rendererSubsystem,
				context.renderResources
			);
		const bool runtimeContextRendererSubsystemAttached = context.engineAttachments.rendererSubsystem == rendererSubsystem;
		reportLine(formatRuntimeRendererSubsystemStats(
			rendererSubsystemStats,
			runtimeRendererAttached,
			runtimeContextRendererSubsystemAttached
		));
		reportLine(formatRuntimeRendererBackendContractStats(rendererSubsystemStats));
	}

	void RuntimeVerificationReport::reportRendererSubsystemCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::RendererSubsystem* rendererSubsystem
	)
	{
		if (!rendererSubsystem)
		{
			return;
		}

		const auto& rendererSubsystemStats = rendererSubsystem->getFrameBridgeStats();
		const bool runtimeContextRendererSubsystemAttached = context.engineAttachments.rendererSubsystem == rendererSubsystem;
		reportLine(formatRuntimeRendererSubsystemCleanupStats(
			rendererSubsystemStats,
			runtimeContextRendererSubsystemAttached
		));
		reportLine(formatRuntimeRendererBackendContractCleanupStats(rendererSubsystemStats));
	}

	void RuntimeVerificationReport::reportEngineWorldCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine
	)
	{
		const auto engineSnapshot = engine.captureLifecycleSnapshot();
		const RuntimeEngineWorldCleanupReportFields fields{
			context.engineAttachments.engine == nullptr,
			context.engineAttachments.engine == &engine,
			context.engineAttachments.engineWorld == nullptr,
			context.engineAttachments.engineWorld != nullptr,
			context.engineAttachments.engineWorldEditable,
			context.engineAttachments.assetSubsystem == nullptr,
			context.engineAttachments.assetSubsystem != nullptr,
			context.engineAttachments.rendererSubsystem == nullptr,
			context.engineAttachments.rendererSubsystem != nullptr
		};
		reportLine(formatRuntimeEngineWorldCleanupStats(engineSnapshot, fields));
	}

	void RuntimeVerificationReport::reportEngineSubsystemCleanup(
		GLframework::AppRuntimeContext& context,
		const GLengine::Engine& engine,
		const GLengine::AssetSubsystem* assetSubsystem,
		const GLengine::RendererSubsystem* rendererSubsystem
	)
	{
		const auto engineSnapshot = engine.captureLifecycleSnapshot();
		const auto* rendererStats = rendererSubsystem ? &rendererSubsystem->getFrameBridgeStats() : nullptr;
		const RuntimeEngineSubsystemCleanupReportFields fields{
			context.engineAttachments.assetSubsystem != nullptr,
			context.engineAttachments.rendererSubsystem != nullptr,
			assetSubsystem != nullptr,
			assetSubsystem && assetSubsystem->isInitialized(),
			assetSubsystem ? assetSubsystem->getRegistry().count() : std::size_t{ 0 },
			rendererSubsystem != nullptr,
			rendererStats
		};
		reportLine(formatRuntimeEngineSubsystemCleanupStats(engineSnapshot, fields));
	}
}
