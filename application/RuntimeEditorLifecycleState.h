#pragma once

#include "../tools/editor/EditorSelectionState.h"

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleState
	{
		GL_EDITOR::SelectionContext selection{};
		GL_EDITOR::EditTransactionLog editTransactions{};
	};
}
