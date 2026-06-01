#pragma once

#include <functional>
#include <string>
#include <vector>

namespace GL_EDITOR
{
	struct EditorPanelContext;
	struct SelectionContext;

	struct SelectionInspectorProviderContext
	{
		const EditorPanelContext& panelContext;
		SelectionContext& selection;
	};

	using SelectionInspectorCanInspect = std::function<bool(const SelectionInspectorProviderContext&)>;
	using SelectionInspectorDraw = std::function<void(SelectionInspectorProviderContext&)>;

	struct SelectionInspectorProvider
	{
		std::string key{};
		SelectionInspectorCanInspect canInspect{};
		SelectionInspectorDraw draw{};
	};

	class SelectionInspectorProviderRegistry
	{
	public:
		bool registerProvider(SelectionInspectorProvider provider);
		const SelectionInspectorProvider* findProvider(const SelectionInspectorProviderContext& context) const;
		bool drawFirst(SelectionInspectorProviderContext& context) const;

	private:
		std::vector<SelectionInspectorProvider> mProviders{};
	};
}
