#include "RuntimeRenderResourceState.h"

#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"
#include "../renderer/renderer.h"

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

GL_RUNTIME::RuntimeRenderResourceView GL_RUNTIME::RuntimeRenderResourceState::readOnlyView() const
{
	return RuntimeRenderResourceView(*this);
}

glm::vec3& GL_RUNTIME::RuntimeRenderResourceState::clearColor()
{
	return mClearColor;
}

const glm::vec3& GL_RUNTIME::RuntimeRenderResourceState::clearColor() const
{
	return mClearColor;
}

std::shared_ptr<GLframework::Renderer>& GL_RUNTIME::RuntimeRenderResourceState::renderer()
{
	return mRenderer;
}

const std::shared_ptr<GLframework::Renderer>& GL_RUNTIME::RuntimeRenderResourceState::renderer() const
{
	return mRenderer;
}

GLframework::RendererFramePassProfile* GL_RUNTIME::RuntimeRenderResourceState::rendererFramePassProfile()
{
	if (!mRenderer)
	{
		return nullptr;
	}

	return &mRenderer->getFramePassProfile();
}

const GLframework::RendererFramePassProfile* GL_RUNTIME::RuntimeRenderResourceState::rendererFramePassProfile() const
{
	if (!mRenderer)
	{
		return nullptr;
	}

	return &mRenderer->getFramePassProfile();
}

std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceState::sceneOffScreen()
{
	return mSceneOffScreen;
}

const std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceState::sceneOffScreen() const
{
	return mSceneOffScreen;
}

std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceState::sceneInScreen()
{
	return mSceneInScreen;
}

const std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceState::sceneInScreen() const
{
	return mSceneInScreen;
}

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

GL_RUNTIME::RuntimeRenderResourceView::RuntimeRenderResourceView(const RuntimeRenderResourceState& state)
	: mState(&state)
{
}

const glm::vec3& GL_RUNTIME::RuntimeRenderResourceView::clearColor() const
{
	return mState->clearColor();
}

const std::shared_ptr<GLframework::Renderer>& GL_RUNTIME::RuntimeRenderResourceView::renderer() const
{
	return mState->renderer();
}

const std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceView::sceneOffScreen() const
{
	return mState->sceneOffScreen();
}

const std::shared_ptr<GLframework::Scene>& GL_RUNTIME::RuntimeRenderResourceView::sceneInScreen() const
{
	return mState->sceneInScreen();
}

const GLframework::FrameRenderTargets& GL_RUNTIME::RuntimeRenderResourceView::frameRenderTargets() const
{
	return mState->frameRenderTargets();
}

const GLframework::PostProcessPass& GL_RUNTIME::RuntimeRenderResourceView::postProcessPass() const
{
	return mState->postProcessPass();
}

const std::shared_ptr<GLframework::Bloom>& GL_RUNTIME::RuntimeRenderResourceView::bloom() const
{
	return mState->bloom();
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceView::screenQuad() const
{
	return mState->screenQuad();
}

const std::shared_ptr<GLframework::ScreenMaterial>& GL_RUNTIME::RuntimeRenderResourceView::screenMaterial() const
{
	return mState->screenMaterial();
}

const std::shared_ptr<GLframework::GrassInstanceMaterial>& GL_RUNTIME::RuntimeRenderResourceView::grassMaterial() const
{
	return mState->grassMaterial();
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceView::skyBoxMesh() const
{
	return mState->skyBoxMesh();
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceView::movePlane() const
{
	return mState->movePlane();
}

const std::shared_ptr<GLframework::Mesh>& GL_RUNTIME::RuntimeRenderResourceView::textD() const
{
	return mState->textD();
}

const std::shared_ptr<GLframework::PhongCSMShadowMaterial>& GL_RUNTIME::RuntimeRenderResourceView::csmShadowMaterial() const
{
	return mState->csmShadowMaterial();
}
