#pragma once

#include "SelectionInspectorProviderRegistry.h"

namespace GL_EDITOR
{
	void registerDefaultSelectionInspectorProviders(SelectionInspectorProviderRegistry& registry);
	const SelectionInspectorProviderRegistry& getDefaultSelectionInspectorProviderRegistry();
}
