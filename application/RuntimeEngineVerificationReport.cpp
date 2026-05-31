#include "RuntimeEngineVerificationReport.h"

#include "../engine/EngineLifecycleSnapshot.h"
#include "../engine/RendererSubsystemFrameBridgeStats.h"

namespace
{
	std::string yesNo(bool value)
	{
		return value ? "yes" : "no";
	}

	const char* engineRunModeName(GLengine::EngineRunMode runMode)
	{
		switch (runMode)
		{
		case GLengine::EngineRunMode::Editor:
			return "Editor";
		case GLengine::EngineRunMode::Game:
			return "Game";
		case GLengine::EngineRunMode::Verification:
			return "Verification";
		default:
			return "Unknown";
		}
	}

	const GLengine::EngineSubsystemLifecycleSummary* findSubsystemSummary(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const std::string& name
	)
	{
		for (const auto& summary : snapshot.subsystemSummaries)
		{
			if (summary.name == name)
			{
				return &summary;
			}
		}

		return nullptr;
	}

	int subsystemTickCount(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const std::string& name
	)
	{
		const auto* summary = findSubsystemSummary(snapshot, name);
		return summary ? summary->tickCount : 0;
	}

	std::string formatSubsystemSummaryNames(const GLengine::EngineLifecycleSnapshot& snapshot)
	{
		std::string result{ "none" };
		for (const auto& summary : snapshot.subsystemSummaries)
		{
			if (result == "none")
			{
				result.clear();
			}
			else
			{
				result += "|";
			}
			result += summary.name.empty() ? "unnamed" : summary.name;
		}
		return result;
	}

	std::string formatSubsystemSummaryInitialized(const GLengine::EngineLifecycleSnapshot& snapshot)
	{
		std::string result{ "none" };
		for (const auto& summary : snapshot.subsystemSummaries)
		{
			if (result == "none")
			{
				result.clear();
			}
			else
			{
				result += "|";
			}
			result += yesNo(summary.initialized);
		}
		return result;
	}

	std::string formatSubsystemSummaryTicks(const GLengine::EngineLifecycleSnapshot& snapshot)
	{
		std::string result{ "none" };
		for (const auto& summary : snapshot.subsystemSummaries)
		{
			if (result == "none")
			{
				result.clear();
			}
			else
			{
				result += "|";
			}
			result += std::to_string(summary.tickCount);
		}
		return result;
	}
}

namespace GL_RUNTIME
{
	std::string formatRuntimeEngineLifecycleSnapshotStats(
		const GLengine::EngineLifecycleSnapshot& snapshot
	)
	{
		return "Runtime engine lifecycle snapshot stats: initialized="
			+ yesNo(snapshot.initialized)
			+ ", runMode=" + engineRunModeName(snapshot.runMode)
			+ ", viewport=" + std::to_string(snapshot.viewportWidth)
			+ "x" + std::to_string(snapshot.viewportHeight)
			+ ", activeWorld=" + yesNo(snapshot.activeWorldPresent)
			+ ", activeWorldPlaying=" + yesNo(snapshot.activeWorldPlaying)
			+ ", activeWorldActors=" + std::to_string(snapshot.activeWorldActorCount)
			+ ", activeWorldTicks=" + std::to_string(snapshot.activeWorldTickCount)
			+ ", engineTicks=" + std::to_string(snapshot.engineTickCount)
			+ ", subsystemCount=" + std::to_string(snapshot.subsystemCount)
			+ ", initializedSubsystems=" + std::to_string(snapshot.initializedSubsystemCount)
			+ ", tickedSubsystems=" + std::to_string(snapshot.tickedSubsystemCount)
			+ ", timeSeconds=" + std::to_string(snapshot.timeSeconds)
			+ ", deltaSeconds=" + std::to_string(snapshot.deltaSeconds);
	}

	std::string formatRuntimeEngineSubsystemSummaryStats(
		const GLengine::EngineLifecycleSnapshot& snapshot
	)
	{
		return "Runtime engine subsystem summary stats: subsystems="
			+ std::to_string(snapshot.subsystemCount)
			+ ", initializedSubsystems=" + std::to_string(snapshot.initializedSubsystemCount)
			+ ", tickedSubsystems=" + std::to_string(snapshot.tickedSubsystemCount)
			+ ", names=" + formatSubsystemSummaryNames(snapshot)
			+ ", initialized=" + formatSubsystemSummaryInitialized(snapshot)
			+ ", ticks=" + formatSubsystemSummaryTicks(snapshot);
	}

	std::string formatRuntimeEngineTickStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		bool runtimeContextEngineAttached
	)
	{
		return "Runtime engine tick stats: initialized="
			+ yesNo(snapshot.initialized)
			+ ", runtimeContextEngineAttached=" + yesNo(runtimeContextEngineAttached)
			+ ", activeWorld=" + yesNo(snapshot.activeWorldPresent)
			+ ", activeWorldPlaying=" + yesNo(snapshot.activeWorldPlaying)
			+ ", timeSeconds=" + std::to_string(snapshot.timeSeconds)
			+ ", deltaSeconds=" + std::to_string(snapshot.deltaSeconds);
	}

	std::string formatRuntimeSubsystemHealthStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		bool contextWorldMatchesActive
	)
	{
		return "Runtime subsystem health stats: engineTicks="
			+ std::to_string(snapshot.engineTickCount)
			+ ", worldTicks=" + std::to_string(snapshot.activeWorldTickCount)
			+ ", assetSubsystemTicks=" + std::to_string(subsystemTickCount(snapshot, "AssetSubsystem"))
			+ ", rendererSubsystemTicks=" + std::to_string(subsystemTickCount(snapshot, "RendererSubsystem"))
			+ ", contextWorldMatchesActive=" + yesNo(contextWorldMatchesActive)
			+ ", worldActors=" + std::to_string(snapshot.activeWorldActorCount);
	}

	std::string formatRuntimeEngineWorldCleanupStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const RuntimeEngineWorldCleanupReportFields& fields
	)
	{
		return "Runtime engine world cleanup stats: engineInitialized="
			+ yesNo(snapshot.initialized)
			+ ", runtimeContextEngineNull=" + yesNo(fields.runtimeContextEngineNull)
			+ ", runtimeContextEngineAttached=" + yesNo(fields.runtimeContextEngineAttached)
			+ ", activeWorld=" + yesNo(snapshot.activeWorldPresent)
			+ ", activeWorldPlaying=" + yesNo(snapshot.activeWorldPlaying)
			+ ", runtimeContextWorldNull=" + yesNo(fields.runtimeContextWorldNull)
			+ ", runtimeContextWorldAttached=" + yesNo(fields.runtimeContextWorldAttached)
			+ ", runtimeContextWorldEditable=" + yesNo(fields.runtimeContextWorldEditable)
			+ ", runtimeContextAssetSubsystemNull=" + yesNo(fields.runtimeContextAssetSubsystemNull)
			+ ", runtimeContextAssetSubsystemAttached=" + yesNo(fields.runtimeContextAssetSubsystemAttached)
			+ ", runtimeContextRendererSubsystemNull=" + yesNo(fields.runtimeContextRendererSubsystemNull)
			+ ", runtimeContextRendererSubsystemAttached=" + yesNo(fields.runtimeContextRendererSubsystemAttached)
			+ ", engineTicks=" + std::to_string(snapshot.engineTickCount)
			+ ", engineTimeSeconds=" + std::to_string(snapshot.timeSeconds)
			+ ", engineDeltaSeconds=" + std::to_string(snapshot.deltaSeconds);
	}

	std::string formatRuntimeEngineSubsystemCleanupStats(
		const GLengine::EngineLifecycleSnapshot& snapshot,
		const RuntimeEngineSubsystemCleanupReportFields& fields
	)
	{
		const auto* rendererStats = fields.rendererStats;
		return "Runtime engine subsystem cleanup stats: runtimeContextAssetSubsystemAttached="
			+ yesNo(fields.runtimeContextAssetSubsystemAttached)
			+ ", runtimeContextRendererSubsystemAttached=" + yesNo(fields.runtimeContextRendererSubsystemAttached)
			+ ", assetSubsystemPresent=" + yesNo(fields.assetSubsystemPresent)
			+ ", assetSubsystemInitialized=" + yesNo(fields.assetSubsystemInitialized)
			+ ", assetSubsystemTicks=" + std::to_string(subsystemTickCount(snapshot, "AssetSubsystem"))
			+ ", assetRegistryAssets=" + std::to_string(fields.assetRegistryAssetCount)
			+ ", rendererSubsystemPresent=" + yesNo(fields.rendererSubsystemPresent)
			+ ", rendererSubsystemInitialized=" + yesNo(rendererStats && rendererStats->initialized)
			+ ", rendererSubsystemTicks=" + std::to_string(subsystemTickCount(snapshot, "RendererSubsystem"))
			+ ", rendererHasRenderer=" + yesNo(rendererStats && rendererStats->hasRenderer)
			+ ", rendererBackendAttached=" + yesNo(rendererStats && rendererStats->rendererBackendAttached)
			+ ", rendererBackendReady=" + yesNo(rendererStats && rendererStats->rendererBackendReady)
			+ ", rendererBackendKey=" + (rendererStats ? rendererStats->rendererBackendKey : std::string{ "none" })
			+ ", rendererBackendState=" + (rendererStats ? rendererStats->rendererBackendState : std::string{ "detached" })
			+ ", rendererBackendOwnerKey=" + (rendererStats ? rendererStats->rendererBackendOwnerKey : std::string{ "none" })
			+ ", rendererBackendOwnership=" + (rendererStats ? rendererStats->rendererBackendOwnership : std::string{ "detached" })
			+ ", rendererBackendRegistryKey=" + (rendererStats ? rendererStats->rendererBackendRegistryKey : std::string{ "none" })
			+ ", rendererBackendRegistryCount=" + std::to_string(rendererStats ? rendererStats->rendererBackendRegistryCount : 0)
			+ ", rendererBackendAttachCount=" + std::to_string(rendererStats ? rendererStats->rendererBackendAttachCount : 0)
			+ ", rendererBackendDetachCount=" + std::to_string(rendererStats ? rendererStats->rendererBackendDetachCount : 0)
			+ ", snapshotSubsystems=" + std::to_string(snapshot.subsystemCount)
			+ ", snapshotInitializedSubsystems=" + std::to_string(snapshot.initializedSubsystemCount)
			+ ", snapshotTickedSubsystems=" + std::to_string(snapshot.tickedSubsystemCount)
			+ ", snapshotSubsystemNames=" + formatSubsystemSummaryNames(snapshot)
			+ ", snapshotSubsystemInitialized=" + formatSubsystemSummaryInitialized(snapshot)
			+ ", snapshotSubsystemTicks=" + formatSubsystemSummaryTicks(snapshot);
	}
}
