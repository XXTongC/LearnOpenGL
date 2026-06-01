#pragma once

#include <memory>

#include "../third_party/glm/glm.hpp"

namespace GLframework
{
	class Bloom;
	class FrameRenderTargets;
	class GrassInstanceMaterial;
	class Mesh;
	class PhongCSMShadowMaterial;
	class PostProcessPass;
	class Renderer;
	struct RendererFramePassProfile;
	class Scene;
	class ScreenMaterial;
}

namespace GL_RUNTIME
{
	class RuntimeRenderResourceView;

	struct RuntimeRenderResourceState
	{
		RuntimeRenderResourceState();
		~RuntimeRenderResourceState();

		RuntimeRenderResourceState(const RuntimeRenderResourceState&) = delete;
		RuntimeRenderResourceState& operator=(const RuntimeRenderResourceState&) = delete;
		RuntimeRenderResourceState(RuntimeRenderResourceState&&) noexcept;
		RuntimeRenderResourceState& operator=(RuntimeRenderResourceState&&) noexcept;

		RuntimeRenderResourceView readOnlyView() const;

		glm::vec3& clearColor();
		const glm::vec3& clearColor() const;
		void syncClearColorToRenderer() const;
		std::shared_ptr<GLframework::Renderer>& renderer();
		const std::shared_ptr<GLframework::Renderer>& renderer() const;
		GLframework::RendererFramePassProfile* rendererFramePassProfile();
		const GLframework::RendererFramePassProfile* rendererFramePassProfile() const;
		std::shared_ptr<GLframework::Scene>& sceneOffScreen();
		const std::shared_ptr<GLframework::Scene>& sceneOffScreen() const;
		std::shared_ptr<GLframework::Scene>& sceneInScreen();
		const std::shared_ptr<GLframework::Scene>& sceneInScreen() const;
		GLframework::FrameRenderTargets& frameRenderTargets();
		const GLframework::FrameRenderTargets& frameRenderTargets() const;
		GLframework::PostProcessPass& postProcessPass();
		const GLframework::PostProcessPass& postProcessPass() const;
		std::shared_ptr<GLframework::Bloom>& bloom();
		const std::shared_ptr<GLframework::Bloom>& bloom() const;
		std::shared_ptr<GLframework::Mesh>& screenQuad();
		const std::shared_ptr<GLframework::Mesh>& screenQuad() const;
		std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial();
		const std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial() const;
		std::shared_ptr<GLframework::GrassInstanceMaterial>& grassMaterial();
		const std::shared_ptr<GLframework::GrassInstanceMaterial>& grassMaterial() const;
		std::shared_ptr<GLframework::Mesh>& skyBoxMesh();
		const std::shared_ptr<GLframework::Mesh>& skyBoxMesh() const;
		std::shared_ptr<GLframework::Mesh>& movePlane();
		const std::shared_ptr<GLframework::Mesh>& movePlane() const;
		std::shared_ptr<GLframework::Mesh>& textD();
		const std::shared_ptr<GLframework::Mesh>& textD() const;
		std::shared_ptr<GLframework::PhongCSMShadowMaterial>& csmShadowMaterial();
		const std::shared_ptr<GLframework::PhongCSMShadowMaterial>& csmShadowMaterial() const;

	private:
		glm::vec3 mClearColor{};
		std::shared_ptr<GLframework::Renderer> mRenderer{ nullptr };
		std::shared_ptr<GLframework::Scene> mSceneOffScreen{ nullptr };
		std::shared_ptr<GLframework::Scene> mSceneInScreen{ nullptr };
		std::shared_ptr<GLframework::Bloom> mBloom{ nullptr };
		std::shared_ptr<GLframework::Mesh> mScreenQuad{ nullptr };
		std::shared_ptr<GLframework::ScreenMaterial> mScreenMaterial{ nullptr };
		std::shared_ptr<GLframework::GrassInstanceMaterial> mGrassMaterial{ nullptr };
		std::shared_ptr<GLframework::Mesh> mSkyBoxMesh{ nullptr };
		std::shared_ptr<GLframework::Mesh> mMovePlane{ nullptr };
		std::shared_ptr<GLframework::Mesh> mTextD{ nullptr };
		std::shared_ptr<GLframework::PhongCSMShadowMaterial> mCsmShadowMaterial{ nullptr };
		std::unique_ptr<GLframework::FrameRenderTargets> mFrameRenderTargets{};
		std::unique_ptr<GLframework::PostProcessPass> mPostProcessPass{};
	};

	class RuntimeRenderResourceView
	{
	public:
		explicit RuntimeRenderResourceView(const RuntimeRenderResourceState& state);

		const glm::vec3& clearColor() const;
		const std::shared_ptr<GLframework::Renderer>& renderer() const;
		const std::shared_ptr<GLframework::Scene>& sceneOffScreen() const;
		const std::shared_ptr<GLframework::Scene>& sceneInScreen() const;
		const GLframework::FrameRenderTargets& frameRenderTargets() const;
		const GLframework::PostProcessPass& postProcessPass() const;
		const std::shared_ptr<GLframework::Bloom>& bloom() const;
		const std::shared_ptr<GLframework::Mesh>& screenQuad() const;
		const std::shared_ptr<GLframework::ScreenMaterial>& screenMaterial() const;
		const std::shared_ptr<GLframework::GrassInstanceMaterial>& grassMaterial() const;
		const std::shared_ptr<GLframework::Mesh>& skyBoxMesh() const;
		const std::shared_ptr<GLframework::Mesh>& movePlane() const;
		const std::shared_ptr<GLframework::Mesh>& textD() const;
		const std::shared_ptr<GLframework::PhongCSMShadowMaterial>& csmShadowMaterial() const;

	private:
		const RuntimeRenderResourceState* mState{ nullptr };
	};
}
