#pragma once

#include "engine/RendererSubsystemFrameBridgeStats.h"

namespace GLengine
{
	struct EngineContext;
	struct RendererFrameIntent;
	struct RendererFrameResult;
	struct RendererSubsystemBackendSlotSnapshot;

	class RendererSubsystemFrameBridgeState
	{
	public:
		const RendererSubsystemFrameBridgeStats& getStats() const;

		void setRendererObservation(bool hasRenderer, int observedRendererPasses);
		void recordRendererBackendAttachmentChange(bool detached, bool attached);
		void recordRenderFrameBridgeCall();
		void recordRendererBackendFrameCall();
		void recordRendererBackendReadyFrame();
		void recordRendererBackendNotReadyFrame();
		void recordBeginFrame();
		void recordCompletedFrame();

		void applyFrameIntent(const RendererFrameIntent& frameIntent);
		void applyFrameResult(const RendererFrameResult& frameResult);
		void refreshRendererBackendStats(
			const RendererSubsystemBackendSlotSnapshot& backendSnapshot
		);
		void refreshFrameBridgeStats(
			const EngineContext& context,
			bool initialized,
			bool hasRenderer,
			bool frameBridgeActive,
			int observedRendererPasses,
			const RendererSubsystemBackendSlotSnapshot& backendSnapshot
		);

	private:
		RendererSubsystemFrameBridgeStats mStats{};
	};
}
