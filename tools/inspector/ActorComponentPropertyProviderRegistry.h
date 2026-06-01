#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "PropertySchema.h"

namespace GLengine
{
	class ActorComponent;
}

namespace GL_EDITOR
{
	class EditTransactionLog;

	struct ActorComponentPropertyProviderContext
	{
		GLengine::ActorComponent& component;
		bool engineWorldEditable{ false };
		EditTransactionLog* editTransactions{ nullptr };
	};

	using ActorComponentPropertyCanBuild = std::function<bool(const ActorComponentPropertyProviderContext&)>;
	using ActorComponentPropertyBuild = std::function<void(PropertyBuilder&, const ActorComponentPropertyProviderContext&)>;

	struct ActorComponentPropertyProvider
	{
		std::string key{};
		ActorComponentPropertyCanBuild canBuild{};
		ActorComponentPropertyBuild build{};
	};

	class ActorComponentPropertyProviderRegistry
	{
	public:
		bool registerProvider(ActorComponentPropertyProvider provider);
		std::size_t buildMatching(PropertyBuilder& builder, const ActorComponentPropertyProviderContext& context) const;

	private:
		std::vector<ActorComponentPropertyProvider> mProviders{};
	};
}
