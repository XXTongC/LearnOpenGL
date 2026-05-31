#pragma once

#include <memory>

namespace GL_EDITOR
{
	class EditTransactionLog;
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

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl{};
	};
}
