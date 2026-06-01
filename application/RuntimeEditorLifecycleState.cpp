#include "RuntimeEditorLifecycleState.h"

#include "../tools/editor/EditorSelectionState.h"
#include "../tools/editor/EditorUiModuleRegistry.h"

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleState::Impl
	{
		GL_EDITOR::SelectionContext selection{};
		GL_EDITOR::EditTransactionLog editTransactions{};
		GL_EDITOR::EditorUiModuleRegistries editorUiModules{ GL_EDITOR::buildDefaultEditorUiModuleRegistries() };
	};

	RuntimeEditorLifecycleState::RuntimeEditorLifecycleState()
		: mImpl(std::make_unique<Impl>())
	{
	}

	RuntimeEditorLifecycleState::~RuntimeEditorLifecycleState() = default;

	GL_EDITOR::SelectionContext& RuntimeEditorLifecycleState::selection()
	{
		return mImpl->selection;
	}

	const GL_EDITOR::SelectionContext& RuntimeEditorLifecycleState::selection() const
	{
		return mImpl->selection;
	}

	GL_EDITOR::EditTransactionLog& RuntimeEditorLifecycleState::editTransactions()
	{
		return mImpl->editTransactions;
	}

	const GL_EDITOR::EditTransactionLog& RuntimeEditorLifecycleState::editTransactions() const
	{
		return mImpl->editTransactions;
	}

	const GL_EDITOR::EditorUiModuleRegistries& RuntimeEditorLifecycleState::editorUiModules() const
	{
		return mImpl->editorUiModules;
	}
}
