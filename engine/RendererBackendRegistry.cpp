#include "engine/RendererBackendRegistry.h"

#include <algorithm>
#include <utility>

using namespace GLengine;

RendererBackendRegistry::RendererBackendRegistry(std::vector<RendererBackendRegistration> registrations)
	: mRegistrations(std::move(registrations))
{
}

const std::vector<RendererBackendRegistration>& RendererBackendRegistry::getRegisteredBackends() const
{
	return mRegistrations;
}

int RendererBackendRegistry::getBackendCount() const
{
	return static_cast<int>(mRegistrations.size());
}

bool RendererBackendRegistry::isRegisteredBackendKey(std::string_view backendKey) const
{
	return std::any_of(mRegistrations.begin(), mRegistrations.end(), [&backendKey](const RendererBackendRegistration& backend)
	{
		return std::string_view{ backend.key } == backendKey;
	});
}

std::string RendererBackendRegistry::getDefaultBackendKey(std::string_view fallbackKey) const
{
	const auto defaultBackend = std::find_if(mRegistrations.begin(), mRegistrations.end(), [](const RendererBackendRegistration& backend)
	{
		return backend.defaultBackend && !backend.key.empty();
	});
	if (defaultBackend != mRegistrations.end())
	{
		return defaultBackend->key;
	}

	const auto firstBackend = std::find_if(mRegistrations.begin(), mRegistrations.end(), [](const RendererBackendRegistration& backend)
	{
		return !backend.key.empty();
	});
	return firstBackend != mRegistrations.end() ? firstBackend->key : std::string{ fallbackKey };
}

RendererBackendSelection RendererBackendRegistry::resolveBackendSelection(
	std::string_view requestedKey,
	std::string_view fallbackKey
) const
{
	RendererBackendSelection selection{};
	selection.requestedKey = requestedKey.empty() ? "none" : std::string{ requestedKey };
	selection.defaultKey = getDefaultBackendKey(fallbackKey);
	selection.registryBackendCount = getBackendCount();
	selection.usedDefault = requestedKey.empty();

	const std::string candidateKey = requestedKey.empty() ? selection.defaultKey : std::string{ requestedKey };
	if (isRegisteredBackendKey(candidateKey))
	{
		selection.selectedKey = candidateKey;
		selection.registered = true;
	}
	return selection;
}

RendererBackendAttachmentDesc RendererBackendRegistry::makeAttachmentDesc(
	std::string_view registryKey,
	std::string_view ownerKey,
	std::string_view ownership
) const
{
	return {
		ownerKey.empty() ? "external" : std::string{ ownerKey },
		ownership.empty() ? "external" : std::string{ ownership },
		registryKey.empty() ? getDefaultBackendKey() : std::string{ registryKey },
		getBackendCount()
	};
}
