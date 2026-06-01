#include "RuntimeRenderResourceState.h"

#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"

GL_RUNTIME::RuntimeRenderResourceState::RuntimeRenderResourceState()
	: mFrameRenderTargets(std::make_unique<GLframework::FrameRenderTargets>()),
	  mPostProcessPass(std::make_unique<GLframework::PostProcessPass>())
{
}

GL_RUNTIME::RuntimeRenderResourceState::~RuntimeRenderResourceState() = default;

GL_RUNTIME::RuntimeRenderResourceState::RuntimeRenderResourceState(RuntimeRenderResourceState&&) noexcept = default;

GL_RUNTIME::RuntimeRenderResourceState& GL_RUNTIME::RuntimeRenderResourceState::operator=(
	RuntimeRenderResourceState&&
) noexcept = default;

GLframework::FrameRenderTargets& GL_RUNTIME::RuntimeRenderResourceState::frameRenderTargets()
{
	return *mFrameRenderTargets;
}

const GLframework::FrameRenderTargets& GL_RUNTIME::RuntimeRenderResourceState::frameRenderTargets() const
{
	return *mFrameRenderTargets;
}

GLframework::PostProcessPass& GL_RUNTIME::RuntimeRenderResourceState::postProcessPass()
{
	return *mPostProcessPass;
}

const GLframework::PostProcessPass& GL_RUNTIME::RuntimeRenderResourceState::postProcessPass() const
{
	return *mPostProcessPass;
}

std::shared_ptr<GLframework::Bloom>& GL_RUNTIME::RuntimeRenderResourceState::bloom()
{
	return mBloom;
}

const std::shared_ptr<GLframework::Bloom>& GL_RUNTIME::RuntimeRenderResourceState::bloom() const
{
	return mBloom;
}
