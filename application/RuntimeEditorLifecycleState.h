#pragma once

#include <memory>

namespace GL_EDITOR
{
	class EditTransactionLog;
	struct EditorUiModuleCompositionPolicy;
	struct EditorUiModuleRegistries;
	struct SelectionContext;
}

namespace GL_RUNTIME
{
	class RuntimeEditorLifecycleState
	{
	public:
		RuntimeEditorLifecycleState();
		~RuntimeEditorLifecycleState();

		RuntimeEditorLifecycleState(const RuntimeEditorLifecycleState&) = delete;
		RuntimeEditorLifecycleState& operator=(const RuntimeEditorLifecycleState&) = delete;

		GL_EDITOR::SelectionContext& selection();
		const GL_EDITOR::SelectionContext& selection() const;

		GL_EDITOR::EditTransactionLog& editTransactions();
		const GL_EDITOR::EditTransactionLog& editTransactions() const;

		void configureEditorUiModules(const GL_EDITOR::EditorUiModuleCompositionPolicy& policy);
		const GL_EDITOR::EditorUiModuleRegistries& editorUiModules() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl{};
	};
}
