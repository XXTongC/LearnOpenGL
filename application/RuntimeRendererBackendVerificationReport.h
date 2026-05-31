#pragma once

#include <string>

namespace GLengine
{
	struct RendererSubsystemFrameBridgeStats;
}

namespace GL_RUNTIME
{
	std::string formatRuntimeRendererSubsystemStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats,
		bool runtimeRendererAttached,
		bool runtimeContextRendererSubsystemAttached
	);

	std::string formatRuntimeRendererBackendContractStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats
	);

	std::string formatRuntimeRendererSubsystemCleanupStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats,
		bool runtimeContextRendererSubsystemAttached
	);

	std::string formatRuntimeRendererBackendContractCleanupStats(
		const GLengine::RendererSubsystemFrameBridgeStats& stats
	);
}
