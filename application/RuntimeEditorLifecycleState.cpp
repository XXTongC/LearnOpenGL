#include "RuntimeEditorLifecycleState.h"

#include "../tools/editor/EditorSelectionState.h"
#include "../tools/editor/EditorUiModuleComposition.h"
#include "../tools/editor/EditorUiModulePolicyDiagnostics.h"
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

		GL_EDITOR::EditorUiModulePolicySnapshot makeEditorUiModulePolicySnapshot(
			const GL_EDITOR::EditorUiModuleCompositionPolicy& policy
		)
		{
			GL_EDITOR::EditorUiModulePolicySnapshot snapshot{};
			snapshot.includeCoreEditorUi = policy.includeCoreEditorUi;
			snapshot.includeSampleEditorUi = policy.includeSampleEditorUi;
			return snapshot;
		}

		GL_EDITOR::EditorUiModulePolicyDiagnostics makeEditorUiModulePolicyDiagnostics(
			const GL_EDITOR::EditorUiModuleCompositionPolicy& activePolicy
		)
		{
			GL_EDITOR::EditorUiModulePolicyDiagnostics diagnostics{};
			diagnostics.activePolicy = makeEditorUiModulePolicySnapshot(activePolicy);
			diagnostics.pendingPolicy = diagnostics.activePolicy;
			diagnostics.lastAppliedPolicy = diagnostics.activePolicy;
			diagnostics.hasLastAppliedPolicy = true;
			diagnostics.registryBuildCount = 1;
			return diagnostics;
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
		GL_EDITOR::EditorUiModulePolicyDiagnostics editorUiModuleDiagnostics{
			makeEditorUiModulePolicyDiagnostics(editorUiModulePolicy)
		};
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
		mImpl->editorUiModuleDiagnostics.activePolicy = makeEditorUiModulePolicySnapshot(policy);
		mImpl->editorUiModuleDiagnostics.lastAppliedPolicy = mImpl->editorUiModuleDiagnostics.activePolicy;
		mImpl->editorUiModuleDiagnostics.hasLastAppliedPolicy = true;
		++mImpl->editorUiModuleDiagnostics.registryBuildCount;
		return true;
	}

	bool RuntimeEditorLifecycleState::requestEditorUiModuleReconfiguration(
		const GL_EDITOR::EditorUiModuleProfile& profile
	)
	{
		const auto policy = makeEditorUiModulePolicy(profile);
		mImpl->pendingEditorUiModulePolicy = policy;
		mImpl->hasPendingEditorUiModulePolicy = true;
		mImpl->editorUiModuleDiagnostics.pendingPolicy = makeEditorUiModulePolicySnapshot(policy);
		mImpl->editorUiModuleDiagnostics.hasPendingPolicy = true;
		++mImpl->editorUiModuleDiagnostics.reapplyRequestCount;
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
		mImpl->editorUiModuleDiagnostics.hasPendingPolicy = false;
		++mImpl->editorUiModuleDiagnostics.reapplyApplyCount;
		const bool rebuilt = configureEditorUiModules(policy);
		mImpl->editorUiModuleDiagnostics.lastAppliedPolicy = makeEditorUiModulePolicySnapshot(policy);
		mImpl->editorUiModuleDiagnostics.hasLastAppliedPolicy = true;
		mImpl->editorUiModuleDiagnostics.lastApplyRebuiltRegistries = rebuilt;
		return rebuilt;
	}

	const GL_EDITOR::EditorUiModuleRegistries& RuntimeEditorLifecycleState::editorUiModules() const
	{
		return mImpl->editorUiModules;
	}

	const GL_EDITOR::EditorUiModulePolicyDiagnostics& RuntimeEditorLifecycleState::editorUiModulePolicyDiagnostics() const
	{
		return mImpl->editorUiModuleDiagnostics;
	}
}
