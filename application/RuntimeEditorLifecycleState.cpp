#include "RuntimeEditorLifecycleState.h"

#include "../tools/editor/EditorSelectionState.h"
#include "../tools/editor/EditorUiModuleComposition.h"
#include "../tools/editor/EditorUiModuleProfile.h"
#include "../tools/editor/EditorUiModuleRegistry.h"

namespace GL_RUNTIME
{
	namespace
	{
		bool isSameEditorUiModulePolicy(
			const GL_EDITOR::EditorUiModuleCompositionPolicy& lhs,
			const GL_EDITOR::EditorUiModuleCompositionPolicy& rhs
		)
		{
			return lhs.includeCoreEditorUi == rhs.includeCoreEditorUi
				&& lhs.includeSampleEditorUi == rhs.includeSampleEditorUi;
		}

		GL_EDITOR::EditorUiModuleRegistries buildEditorUiModuleRegistries(
			const GL_EDITOR::EditorUiModuleCompositionPolicy& policy
		)
		{
			return GL_EDITOR::buildEditorUiModuleRegistries(GL_EDITOR::buildEditorUiModuleList(policy));
		}

		GL_EDITOR::EditorUiModuleCompositionPolicy makeEditorUiModulePolicy(
			const GL_EDITOR::EditorUiModuleProfile& profile
		)
		{
			GL_EDITOR::EditorUiModuleCompositionPolicy policy{};
			policy.includeCoreEditorUi = profile.enableCoreEditorUiModule;
			policy.includeSampleEditorUi = profile.enableSampleEditorUiModule;
			return policy;
		}
	}

	struct RuntimeEditorLifecycleState::Impl
	{
		GL_EDITOR::SelectionContext selection{};
		GL_EDITOR::EditTransactionLog editTransactions{};
		GL_EDITOR::EditorUiModuleCompositionPolicy editorUiModulePolicy{ GL_EDITOR::defaultEditorUiModuleCompositionPolicy() };
		GL_EDITOR::EditorUiModuleRegistries editorUiModules{ buildEditorUiModuleRegistries(editorUiModulePolicy) };
		bool hasPendingEditorUiModulePolicy{ false };
		GL_EDITOR::EditorUiModuleCompositionPolicy pendingEditorUiModulePolicy{};
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

	bool RuntimeEditorLifecycleState::configureEditorUiModules(
		const GL_EDITOR::EditorUiModuleCompositionPolicy& policy
	)
	{
		if (isSameEditorUiModulePolicy(mImpl->editorUiModulePolicy, policy))
		{
			return false;
		}

		mImpl->editorUiModulePolicy = policy;
		mImpl->editorUiModules = buildEditorUiModuleRegistries(policy);
		return true;
	}

	bool RuntimeEditorLifecycleState::requestEditorUiModuleReconfiguration(
		const GL_EDITOR::EditorUiModuleProfile& profile
	)
	{
		const auto policy = makeEditorUiModulePolicy(profile);
		mImpl->pendingEditorUiModulePolicy = policy;
		mImpl->hasPendingEditorUiModulePolicy = true;
		return !isSameEditorUiModulePolicy(mImpl->editorUiModulePolicy, policy);
	}

	bool RuntimeEditorLifecycleState::applyPendingEditorUiModuleReconfiguration()
	{
		if (!mImpl->hasPendingEditorUiModulePolicy)
		{
			return false;
		}

		const auto policy = mImpl->pendingEditorUiModulePolicy;
		mImpl->hasPendingEditorUiModulePolicy = false;
		return configureEditorUiModules(policy);
	}

	const GL_EDITOR::EditorUiModuleRegistries& RuntimeEditorLifecycleState::editorUiModules() const
	{
		return mImpl->editorUiModules;
	}
}
