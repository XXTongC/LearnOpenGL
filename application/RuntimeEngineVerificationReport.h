#pragma once

#include <cstddef>
#include <string>

namespace GLengine
{
	struct EngineLifecycleSnapshot;
	struct RendererSubsystemFrameBridgeStats;
}

namespace GL_RUNTIME
{
	struct RuntimeEngineWorldCleanupReportFields
	{
		bool runtimeContextEngineNull{ false };
		bool runtimeContextEngineAttached{ false };
		bool runtimeContextWorldNull{ false };
		bool runtimeContextWorldAttached{ false };
		bool runtimeContextWorldEditable{ false };
		bool runtimeContextAssetSubsystemNull{ false };
		bool runtimeContextAssetSubsystemAttached{ false };
		bool runtimeContextRendererSubsystemNull{ false };
		bool runtimeContextRendererSubsystemAttached{ false };
	};

	struct RuntimeEngineSubsystemCleanupReportFields
	{
		bool runtimeContextAssetSubsystemAttached{ false };
		bool runtimeContextRendererSubsystemAttached{ false };
		bool assetSubsystemPresent{ false };
		bool assetSubsystemInitialized{ false };
		std::size_t assetRegistryAssetCount{ 0 };
		bool rendererSubsystemPresent{ false };
		const GLengine::RendererSubsystemFrameBridgeStats* rendererStats{ nullptr };
	};

	std::string formatRuntimeEngineLifecycleSnapshotStats(
		const GLengine::EngineLifecycleSnapshot& snapshot
	);

	std::string formatRuntimeEngineSubsystemSummaryStats(
		const GLengine::EngineLifecycleSnapshot& snapshot
	);

	std::string formatRuntimeEngineTickStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		bool runtimeContextEngineAttached
	);

	std::string formatRuntimeSubsystemHealthStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		bool contextWorldMatchesActive
	);

	std::string formatRuntimeEngineWorldCleanupStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const RuntimeEngineWorldCleanupReportFields& fields
	);

	std::string formatRuntimeEngineSubsystemCleanupStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const RuntimeEngineSubsystemCleanupReportFields& fields
	);
}
