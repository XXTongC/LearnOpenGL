#pragma once

#include <memory>

#include "engine/EngineSubsystem.h"

namespace GLframework
{
	class Renderer;
	struct RendererFrameStats;
}

namespace GLengine
{
	class RendererBackend;
	class RendererSubsystemBackendSlot;
	class RendererSubsystemFrameBridgeState;
	class RendererSubsystemFrameExecutionBridge;
	struct RendererBackendAttachmentDesc;
	struct RendererFrameIntent;
	struct RendererSubsystemFrameBridgeStats;

	class RendererSubsystem : public EngineSubsystem
	{
	public:
		explicit RendererSubsystem(GLframework::Renderer* renderer = nullptr);
		~RendererSubsystem() override;

		bool initialize(EngineContext& context) override;
		void tick(EngineContext& context) override;
		void shutdown(EngineContext& context) override;
		const char* getDebugName() const override;
		bool isInitializedForDiagnostics() const override;
		int getTickCountForDiagnostics() const override;

		void setRenderer(GLframework::Renderer* renderer);
		GLframework::Renderer* getRenderer() const;
		bool hasRenderer() const;
		const GLframework::RendererFrameStats* getLastFrameStats() const;
		void setRendererBackend(std::unique_ptr<RendererBackend> rendererBackend);
		void setRendererBackend(
			std::unique_ptr<RendererBackend> rendererBackend,
			RendererBackendAttachmentDesc attachmentDesc
		);
		void clearRendererBackend();
		RendererBackend* getRendererBackend() const;
		bool hasRendererBackend() const;
		void renderFrameBridge(
			const EngineContext& context,
			const RendererFrameIntent& frameIntent
		);
		void beginFrameBridge(const EngineContext& context, const RendererFrameIntent& frameIntent);
		void endFrameBridge(const EngineContext& context, const RendererFrameIntent& frameIntent);
		const RendererSubsystemFrameBridgeStats& getFrameBridgeStats() const;
		int getTickCount() const;

	private:
		bool isRendererBackendReady() const;
		int getObservedRendererPassCount() const;
		void refreshFrameBridgeStats(const EngineContext& context, bool frameBridgeActive);

		GLframework::Renderer* mRenderer{ nullptr };
		std::unique_ptr<RendererSubsystemBackendSlot> mBackendSlot{};
		std::unique_ptr<RendererSubsystemFrameExecutionBridge> mFrameExecutionBridge{};
		std::unique_ptr<RendererSubsystemFrameBridgeState> mFrameBridgeState{};
		int mTickCount{ 0 };
		bool mInitialized{ false };
	};
}
