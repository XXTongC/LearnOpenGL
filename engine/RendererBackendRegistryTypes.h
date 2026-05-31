#pragma once

#include <string>

namespace GLengine
{
	struct RendererBackendAttachmentDesc
	{
		std::string ownerKey{ "none" };
		std::string ownership{ "external" };
		std::string registryKey{ "none" };
		int registryBackendCount{ 0 };
	};

	struct RendererBackendRegistration
	{
		std::string key{};
		std::string displayName{};
		bool defaultBackend{ false };
	};

	struct RendererBackendSelection
	{
		std::string requestedKey{ "none" };
		std::string selectedKey{ "none" };
		std::string defaultKey{ "none" };
		bool registered{ false };
		bool usedDefault{ false };
		int registryBackendCount{ 0 };
	};
}
