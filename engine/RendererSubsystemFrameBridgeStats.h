#pragma once

#include <string>

namespace GLengine
{
	struct RendererSubsystemFrameBridgeStats
	{
		bool initialized{ false };
		bool hasRenderer{ false };
		bool frameBridgeActive{ false };
		bool rendererBackendAttached{ false };
		bool rendererBackendReady{ false };
		bool frameConfigValid{ false };
		unsigned int framebufferWidth{ 0 };
		unsigned int framebufferHeight{ 0 };
		std::string rendererBackendKey{ "none" };
		std::string rendererBackendState{ "detached" };
		std::string rendererBackendOwnerKey{ "none" };
		std::string rendererBackendOwnership{ "detached" };
		std::string rendererBackendRegistryKey{ "none" };
		int rendererBackendRegistryCount{ 0 };
		std::string framePlanKey{ "none" };
		int plannedPassCount{ 0 };
		int executedPassCount{ 0 };
		int skippedPassCount{ 0 };
		int renderFrameBridgeCallCount{ 0 };
		int rendererBackendFrameCallCount{ 0 };
		int rendererBackendAttachCount{ 0 };
		int rendererBackendDetachCount{ 0 };
		int rendererBackendReadyFrameCount{ 0 };
		int rendererBackendNotReadyFrameCount{ 0 };
		int beginFrameCount{ 0 };
		int completedFrameCount{ 0 };
		int observedRendererPasses{ 0 };
		float lastDeltaSeconds{ 0.0f };
		double engineTimeSeconds{ 0.0 };
	};
}
