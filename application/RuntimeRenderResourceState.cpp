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

std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::screenQuad()
{
	return mScreenQuad;
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::screenQuad() const
{
	return mScreenQuad;
}

std::shared_ptr<GLframework::ScreenMaterial>& GL_RUNTIME::RuntimeRenderResourceState::screenMaterial()
{
	return mScreenMaterial;
}

const std::shared_ptr<GLframework::ScreenMaterial>& GL_RUNTIME::RuntimeRenderResourceState::screenMaterial() const
{
	return mScreenMaterial;
}

std::shared_ptr<GLframework::GrassInstanceMaterial>& GL_RUNTIME::RuntimeRenderResourceState::grassMaterial()
{
	return mGrassMaterial;
}

const std::shared_ptr<GLframework::GrassInstanceMaterial>& GL_RUNTIME::RuntimeRenderResourceState::grassMaterial() const
{
	return mGrassMaterial;
}

std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::skyBoxMesh()
{
	return mSkyBoxMesh;
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::skyBoxMesh() const
{
	return mSkyBoxMesh;
}

std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::movePlane()
{
	return mMovePlane;
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::movePlane() const
{
	return mMovePlane;
}

std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::textD()
{
	return mTextD;
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceState::textD() const
{
	return mTextD;
}

std::shared_ptr<GLframework::PhongCSMShadowMaterial>& GL_RUNTIME::RuntimeRenderResourceState::csmShadowMaterial()
{
	return mCsmShadowMaterial;
}

const std::shared_ptr<GLframework::PhongCSMShadowMaterial>& GL_RUNTIME::RuntimeRenderResourceState::csmShadowMaterial() const
{
	return mCsmShadowMaterial;
}
