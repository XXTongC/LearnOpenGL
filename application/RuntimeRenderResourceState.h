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
	class Scene;
	class ScreenMaterial;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState
	{
		RuntimeRenderResourceState();
		~RuntimeRenderResourceState();

		RuntimeRenderResourceState(const RuntimeRenderResourceState&) = delete;
		RuntimeRenderResourceState& operator=(const RuntimeRenderResourceState&) = delete;
		RuntimeRenderResourceState(RuntimeRenderResourceState&&) noexcept;
		RuntimeRenderResourceState& operator=(RuntimeRenderResourceState&&) noexcept;

		std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneOffScreen{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneInScreen{ nullptr };
		std::shared_ptr<GLframework::Mesh> meshPointLight{ nullptr };
		std::shared_ptr<GLframework::GrassInstanceMaterial> grassMaterial{ nullptr };
		std::shared_ptr<GLframework::Mesh> skyBoxMesh{ nullptr };
		std::shared_ptr<GLframework::Mesh> movePlane{ nullptr };
		std::shared_ptr<GLframework::Mesh> textD{ nullptr };
		std::shared_ptr<GLframework::ScreenMaterial> screenMaterial{ nullptr };
		std::shared_ptr<GLframework::PhongCSMShadowMaterial> csmShadowMaterial{ nullptr };
		glm::vec3 clearColor{};

		GLframework::FrameRenderTargets& frameRenderTargets();
		const GLframework::FrameRenderTargets& frameRenderTargets() const;
		GLframework::PostProcessPass& postProcessPass();
		const GLframework::PostProcessPass& postProcessPass() const;
		std::shared_ptr<GLframework::Bloom>& bloom();
		const std::shared_ptr<GLframework::Bloom>& bloom() const;
		std::shared_ptr<GLframework::Mesh>& screenQuad();
		const std::shared_ptr<GLframework::Mesh>& screenQuad() const;

	private:
		std::shared_ptr<GLframework::Bloom> mBloom{ nullptr };
		std::shared_ptr<GLframework::Mesh> mScreenQuad{ nullptr };
		std::unique_ptr<GLframework::FrameRenderTargets> mFrameRenderTargets{};
		std::unique_ptr<GLframework::PostProcessPass> mPostProcessPass{};
	};
}
