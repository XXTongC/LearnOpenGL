#include "RuntimeRendererBackendVerificationReport.h"

#include "../engine/RendererSubsystemFrameBridgeStats.h"

namespace
{
	std::string yesNo(bool value)
	{
		return value ? "yes" : "no";
	}
}

namespace GL_RUNTIME
{
	std::string formatRuntimeRendererSubsystemStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats,
		bool runtimeRendererAttached,
		bool runtimeContextRendererSubsystemAttached
	)
	{
		return "Runtime renderer subsystem stats: initialized="
			+ yesNo(stats.initialized)
			+ ", hasRenderer=" + yesNo(stats.hasRenderer)
			+ ", runtimeRendererAttached=" + yesNo(runtimeRendererAttached)
			+ ", runtimeContextRendererSubsystemAttached=" + yesNo(runtimeContextRendererSubsystemAttached)
			+ ", frameBridgeActive=" + yesNo(stats.frameBridgeActive)
			+ ", rendererBackendAttached=" + yesNo(stats.rendererBackendAttached)
			+ ", rendererBackendReady=" + yesNo(stats.rendererBackendReady)
			+ ", rendererBackendKey=" + stats.rendererBackendKey
			+ ", rendererBackendState=" + stats.rendererBackendState
			+ ", rendererBackendOwnerKey=" + stats.rendererBackendOwnerKey
			+ ", rendererBackendOwnership=" + stats.rendererBackendOwnership
			+ ", rendererBackendRegistryKey=" + stats.rendererBackendRegistryKey
			+ ", rendererBackendRegistryCount=" + std::to_string(stats.rendererBackendRegistryCount)
			+ ", rendererBackendAttachCount=" + std::to_string(stats.rendererBackendAttachCount)
			+ ", rendererBackendDetachCount=" + std::to_string(stats.rendererBackendDetachCount)
			+ ", rendererBackendReadyFrames=" + std::to_string(stats.rendererBackendReadyFrameCount)
			+ ", rendererBackendNotReadyFrames=" + std::to_string(stats.rendererBackendNotReadyFrameCount)
			+ ", frameConfigValid=" + yesNo(stats.frameConfigValid)
			+ ", framebuffer=" + std::to_string(stats.framebufferWidth)
			+ "x" + std::to_string(stats.framebufferHeight)
			+ ", framePlanKey=" + stats.framePlanKey
			+ ", framePasses=" + std::to_string(stats.plannedPassCount)
			+ "/" + std::to_string(stats.executedPassCount)
			+ "/" + std::to_string(stats.skippedPassCount)
			+ ", renderFrameBridgeCalls=" + std::to_string(stats.renderFrameBridgeCallCount)
			+ ", rendererBackendFrameCalls=" + std::to_string(stats.rendererBackendFrameCallCount)
			+ ", beginFrames=" + std::to_string(stats.beginFrameCount)
			+ ", completedFrames=" + std::to_string(stats.completedFrameCount)
			+ ", observedRendererPasses=" + std::to_string(stats.observedRendererPasses)
			+ ", engineTimeSeconds=" + std::to_string(stats.engineTimeSeconds)
			+ ", engineDeltaSeconds=" + std::to_string(stats.lastDeltaSeconds);
	}

	std::string formatRuntimeRendererBackendContractStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats
	)
	{
		return "Runtime renderer backend contract stats: rendererBackendAttached="
			+ yesNo(stats.rendererBackendAttached)
			+ ", rendererBackendReady=" + yesNo(stats.rendererBackendReady)
			+ ", rendererBackendKey=" + stats.rendererBackendKey
			+ ", rendererBackendState=" + stats.rendererBackendState
			+ ", rendererBackendOwnerKey=" + stats.rendererBackendOwnerKey
			+ ", rendererBackendOwnership=" + stats.rendererBackendOwnership
			+ ", rendererBackendRegistryKey=" + stats.rendererBackendRegistryKey
			+ ", rendererBackendRegistryCount=" + std::to_string(stats.rendererBackendRegistryCount)
			+ ", rendererBackendAttachCount=" + std::to_string(stats.rendererBackendAttachCount)
			+ ", rendererBackendDetachCount=" + std::to_string(stats.rendererBackendDetachCount)
			+ ", rendererBackendReadyFrames=" + std::to_string(stats.rendererBackendReadyFrameCount)
			+ ", rendererBackendNotReadyFrames=" + std::to_string(stats.rendererBackendNotReadyFrameCount)
			+ ", frameConfigValid=" + yesNo(stats.frameConfigValid)
			+ ", framebuffer=" + std::to_string(stats.framebufferWidth)
			+ "x" + std::to_string(stats.framebufferHeight)
			+ ", framePlanKey=" + stats.framePlanKey
			+ ", framePasses=" + std::to_string(stats.plannedPassCount)
			+ "/" + std::to_string(stats.executedPassCount)
			+ "/" + std::to_string(stats.skippedPassCount)
			+ ", renderFrameBridgeCalls=" + std::to_string(stats.renderFrameBridgeCallCount)
			+ ", rendererBackendFrameCalls=" + std::to_string(stats.rendererBackendFrameCallCount)
			+ ", beginFrames=" + std::to_string(stats.beginFrameCount)
			+ ", completedFrames=" + std::to_string(stats.completedFrameCount)
			+ ", observedRendererPasses=" + std::to_string(stats.observedRendererPasses)
			+ ", engineTimeSeconds=" + std::to_string(stats.engineTimeSeconds)
			+ ", engineDeltaSeconds=" + std::to_string(stats.lastDeltaSeconds);
	}

	std::string formatRuntimeRendererSubsystemCleanupStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats,
		bool runtimeContextRendererSubsystemAttached
	)
	{
		return "Runtime renderer subsystem cleanup stats: initialized="
			+ yesNo(stats.initialized)
			+ ", runtimeContextRendererSubsystemAttached=" + yesNo(runtimeContextRendererSubsystemAttached)
			+ ", rendererBackendAttached=" + yesNo(stats.rendererBackendAttached)
			+ ", rendererBackendReady=" + yesNo(stats.rendererBackendReady)
			+ ", rendererBackendKey=" + stats.rendererBackendKey
			+ ", rendererBackendState=" + stats.rendererBackendState
			+ ", rendererBackendOwnerKey=" + stats.rendererBackendOwnerKey
			+ ", rendererBackendOwnership=" + stats.rendererBackendOwnership
			+ ", rendererBackendRegistryKey=" + stats.rendererBackendRegistryKey
			+ ", rendererBackendRegistryCount=" + std::to_string(stats.rendererBackendRegistryCount)
			+ ", rendererBackendAttachCount=" + std::to_string(stats.rendererBackendAttachCount)
			+ ", rendererBackendDetachCount=" + std::to_string(stats.rendererBackendDetachCount)
			+ ", rendererBackendReadyFrames=" + std::to_string(stats.rendererBackendReadyFrameCount)
			+ ", rendererBackendNotReadyFrames=" + std::to_string(stats.rendererBackendNotReadyFrameCount);
	}

	std::string formatRuntimeRendererBackendContractCleanupStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats
	)
	{
		return "Runtime renderer backend contract cleanup stats: rendererBackendAttached="
			+ yesNo(stats.rendererBackendAttached)
			+ ", rendererBackendReady=" + yesNo(stats.rendererBackendReady)
			+ ", rendererBackendKey=" + stats.rendererBackendKey
			+ ", rendererBackendState=" + stats.rendererBackendState
			+ ", rendererBackendOwnerKey=" + stats.rendererBackendOwnerKey
			+ ", rendererBackendOwnership=" + stats.rendererBackendOwnership
			+ ", rendererBackendRegistryKey=" + stats.rendererBackendRegistryKey
			+ ", rendererBackendRegistryCount=" + std::to_string(stats.rendererBackendRegistryCount)
			+ ", rendererBackendAttachCount=" + std::to_string(stats.rendererBackendAttachCount)
			+ ", rendererBackendDetachCount=" + std::to_string(stats.rendererBackendDetachCount)
			+ ", rendererBackendReadyFrames=" + std::to_string(stats.rendererBackendReadyFrameCount)
			+ ", rendererBackendNotReadyFrames=" + std::to_string(stats.rendererBackendNotReadyFrameCount);
	}
}
