#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "engine/RendererBackendRegistryTypes.h"

namespace GLengine
{
	class RendererBackendRegistry
	{
	public:
		RendererBackendRegistry() = default;
		explicit RendererBackendRegistry(std::vector<RendererBackendRegistration> registrations);

		const std::vector<RendererBackendRegistration>& getRegisteredBackends() const;
		int getBackendCount() const;
		bool isRegisteredBackendKey(std::string_view backendKey) const;
		std::string getDefaultBackendKey(std::string_view fallbackKey = "none") const;
		RendererBackendSelection resolveBackendSelection(
			std::string_view requestedKey,
			std::string_view fallbackKey = "none"
		) const;
		RendererBackendAttachmentDesc makeAttachmentDesc(
			std::string_view registryKey,
			std::string_view ownerKey,
			std::string_view ownership
		) const;

	private:
		std::vector<RendererBackendRegistration> mRegistrations{};
	};
}
