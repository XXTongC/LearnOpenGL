#pragma once

namespace GL_EDITOR
{
	struct EditorUiModulePolicySnapshot
	{
		bool includeCoreEditorUi{ true };
		bool includeSampleEditorUi{ true };
	};

	struct EditorUiModulePolicyDiagnostics
	{
		EditorUiModulePolicySnapshot activePolicy{};
		EditorUiModulePolicySnapshot pendingPolicy{};
		EditorUiModulePolicySnapshot lastAppliedPolicy{};
		bool hasPendingPolicy{ false };
		bool hasLastAppliedPolicy{ false };
		bool lastApplyRebuiltRegistries{ false };
		int reapplyRequestCount{ 0 };
		int reapplyApplyCount{ 0 };
		int registryBuildCount{ 0 };
	};
}
