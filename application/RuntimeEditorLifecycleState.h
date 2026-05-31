#pragma once

#include "../tools/editor/EditorPanels.h"

namespace GL_RUNTIME
{
	struct RuntimeEditorLifecycleState
	{
		GL_EDITOR::SelectionContext selection{};
		GL_EDITOR::EditTransactionLog editTransactions{};
	};
}
