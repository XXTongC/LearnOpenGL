#include "ActorComponentPropertyProviderRegistry.h"

#include <utility>

namespace GL_EDITOR
{
	bool ActorComponentPropertyProviderRegistry::registerProvider(ActorComponentPropertyProvider provider)
	{
		if (provider.key.empty() || !provider.canBuild || !provider.build)
		{
			return false;
		}

		for (const auto& existingProvider : mProviders)
		{
			if (existingProvider.key == provider.key)
			{
				return false;
			}
		}

		mProviders.push_back(std::move(provider));
		return true;
	}

	std::size_t ActorComponentPropertyProviderRegistry::buildMatching(
		PropertyBuilder& builder,
		const ActorComponentPropertyProviderContext& context
	) const
	{
		std::size_t builtCount = 0;
		for (const auto& provider : mProviders)
		{
			if (provider.canBuild(context))
			{
				provider.build(builder, context);
				++builtCount;
			}
		}

		return builtCount;
	}
}
