#include "SelectionInspectorProviderRegistry.h"

#include <utility>

namespace GL_EDITOR
{
	bool SelectionInspectorProviderRegistry::registerProvider(SelectionInspectorProvider provider)
	{
		if (provider.key.empty() || !provider.canInspect || !provider.draw)
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

	const SelectionInspectorProvider* SelectionInspectorProviderRegistry::findProvider(const SelectionInspectorProviderContext& context) const
	{
		for (const auto& provider : mProviders)
		{
			if (provider.canInspect(context))
			{
				return &provider;
			}
		}

		return nullptr;
	}

	bool SelectionInspectorProviderRegistry::drawFirst(SelectionInspectorProviderContext& context) const
	{
		const auto* provider = findProvider(context);
		if (!provider)
		{
			return false;
		}

		provider->draw(context);
		return true;
	}
}
