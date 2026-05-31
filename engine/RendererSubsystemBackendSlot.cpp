#include "engine/RendererSubsystemBackendSlot.h"

#include <utility>

#include "engine/RendererBackend.h"

using namespace GLengine;

namespace
{
	std::string makeBackendKey(const RendererBackend* backend)
	{
		if (!backend)
		{
			return "none";
		}

		const auto* backendKey = backend->getBackendKey();
		return backendKey && backendKey[0] != '\0' ? backendKey : "none";
	}
}

RendererSubsystemBackendSlot::~RendererSubsystemBackendSlot() = default;

RendererSubsystemBackendSlotChange RendererSubsystemBackendSlot::setBackend(
	std::unique_ptr<RendererBackend> backend,
	RendererBackendAttachmentDesc attachmentDesc
)
{
	const auto* previousBackend = mBackend.get();
	const auto* nextBackend = backend.get();
	RendererSubsystemBackendSlotChange change{};
	if (previousBackend != nextBackend)
	{
		change.detached = previousBackend != nullptr;
		change.attached = nextBackend != nullptr;
	}

	mBackend = std::move(backend);
	mAttachmentDesc = mBackend
		? normalizeAttachmentDesc(attachmentDesc)
		: RendererBackendAttachmentDesc{};
	return change;
}

RendererBackend* RendererSubsystemBackendSlot::getBackend() const
{
	return mBackend.get();
}

bool RendererSubsystemBackendSlot::hasBackend() const
{
	return mBackend != nullptr;
}

bool RendererSubsystemBackendSlot::isBackendReady() const
{
	return mBackend != nullptr && mBackend->isBackendReady();
}

const RendererBackendAttachmentDesc& RendererSubsystemBackendSlot::getAttachmentDesc() const
{
	return mAttachmentDesc;
}

RendererSubsystemBackendSlotSnapshot RendererSubsystemBackendSlot::captureSnapshot() const
{
	const bool attached = mBackend != nullptr;
	RendererSubsystemBackendSlotSnapshot snapshot{};
	snapshot.attached = attached;
	snapshot.ready = isBackendReady();
	snapshot.backendKey = makeBackendKey(mBackend.get());
	snapshot.attachmentDesc = attached ? mAttachmentDesc : RendererBackendAttachmentDesc{};
	return snapshot;
}

RendererBackendAttachmentDesc RendererSubsystemBackendSlot::normalizeAttachmentDesc(
	const RendererBackendAttachmentDesc& attachmentDesc
)
{
	RendererBackendAttachmentDesc normalized{};
	normalized.ownerKey = attachmentDesc.ownerKey.empty()
		? "external"
		: attachmentDesc.ownerKey;
	normalized.ownership = attachmentDesc.ownership.empty()
		? "external"
		: attachmentDesc.ownership;
	normalized.registryKey = attachmentDesc.registryKey.empty()
		? "none"
		: attachmentDesc.registryKey;
	normalized.registryBackendCount = attachmentDesc.registryBackendCount;
	return normalized;
}
