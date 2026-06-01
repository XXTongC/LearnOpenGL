#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "PropertySchema.h"

namespace GLengine
{
	class Actor;
}

namespace GL_EDITOR
{
	struct ActorPropertyProviderContext
	{
		GLengine::Actor& actor;
		bool engineWorldEditable{ false };
	};

	using ActorPropertyCanBuild = std::function<bool(const ActorPropertyProviderContext&)>;
	using ActorPropertyBuild = std::function<void(PropertyBuilder&, const ActorPropertyProviderContext&)>;

	struct ActorPropertyProvider
	{
		std::string key{};
		ActorPropertyCanBuild canBuild{};
		ActorPropertyBuild build{};
	};

	class ActorPropertyProviderRegistry
	{
	public:
		bool registerProvider(ActorPropertyProvider provider);
		std::size_t buildMatching(PropertyBuilder& builder, const ActorPropertyProviderContext& context) const;

	private:
		std::vector<ActorPropertyProvider> mProviders{};
	};
}
