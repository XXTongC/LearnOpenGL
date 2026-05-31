#include "RuntimeRenderResourceState.h"

#include "../renderer/PostProcessPass.h"

GL_RUNTIME::RuntimeRenderResourceState::RuntimeRenderResourceState()
	: mPostProcessPass(std::make_unique<GLframework::PostProcessPass>())
{
}

GL_RUNTIME::RuntimeRenderResourceState::~RuntimeRenderResourceState() = default;

GL_RUNTIME::RuntimeRenderResourceState::RuntimeRenderResourceState(RuntimeRenderResourceState&&) noexcept = default;

GL_RUNTIME::RuntimeRenderResourceState& GL_RUNTIME::RuntimeRenderResourceState::operator=(
	RuntimeRenderResourceState&&
) noexcept = default;

GLframework::PostProcessPass& GL_RUNTIME::RuntimeRenderResourceState::postProcessPass()
{
	return *mPostProcessPass;
}

const GLframework::PostProcessPass& GL_RUNTIME::RuntimeRenderResourceState::postProcessPass() const
{
	return *mPostProcessPass;
}
