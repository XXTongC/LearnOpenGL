#include "MaterialPropertyProviderRegistry.h"

#include <utility>

namespace GL_EDITOR
{
	bool MaterialPropertyProviderRegistry::registerProvider(MaterialPropertyProvider provider)
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

	const MaterialPropertyProvider* MaterialPropertyProviderRegistry::findProvider(const MaterialPropertyProviderContext& context) const
	{
		for (const auto& provider : mProviders)
		{
			if (provider.canBuild(context))
			{
				return &provider;
			}
		}

		return nullptr;
	}

	bool MaterialPropertyProviderRegistry::buildFirst(
		PropertyBuilder& builder,
		const MaterialPropertyProviderContext& context
	) const
	{
		const auto* provider = findProvider(context);
		if (!provider)
		{
			return false;
		}

		provider->build(builder, context);
		return true;
	}
}
