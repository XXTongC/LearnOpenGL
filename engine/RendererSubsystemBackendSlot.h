#pragma once

#include <memory>
#include <string>

#include "engine/RendererBackend.h"
#include "engine/RendererBackendRegistryTypes.h"

namespace GLengine
{
	struct RendererSubsystemBackendSlotChange
	{
		bool detached{ false };
		bool attached{ false };
	};

	struct RendererSubsystemBackendSlotSnapshot
	{
		bool attached{ false };
		bool ready{ false };
		std::string backendKey{ "none" };
		RendererBackendAttachmentDesc attachmentDesc{};
	};

	class RendererSubsystemBackendSlot
	{
	public:
		RendererSubsystemBackendSlotChange setBackend(
			std::unique_ptr<RendererBackend> backend,
			RendererBackendAttachmentDesc attachmentDesc = {}
		);

		RendererBackend* getBackend() const;
		bool hasBackend() const;
		bool isBackendReady() const;
		const RendererBackendAttachmentDesc& getAttachmentDesc() const;
		RendererSubsystemBackendSlotSnapshot captureSnapshot() const;

	private:
		static RendererBackendAttachmentDesc normalizeAttachmentDesc(
			const RendererBackendAttachmentDesc& attachmentDesc
		);

		std::unique_ptr<RendererBackend> mBackend{};
		RendererBackendAttachmentDesc mAttachmentDesc{};
	};
}
