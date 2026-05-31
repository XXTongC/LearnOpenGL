#pragma once

#include <memory>

#include "../renderer/FrameRenderTargets.h"
#include "../renderer/PostProcessPass.h"
#include "../third_party/glm/glm.hpp"

namespace GLframework
{
	class Bloom;
	class GrassInstanceMaterial;
	class Mesh;
	class PhongCSMShadowMaterial;
	class Renderer;
	class Scene;
	class ScreenMaterial;
}

namespace GL_RUNTIME
{
	struct RuntimeRenderResourceState
	{
		std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneOffScreen{ nullptr };
		std::shared_ptr<GLframework::Scene> sceneInScreen{ nullptr };
		std::shared_ptr<GLframework::Mesh> meshPointLight{ nullptr };
		std::shared_ptr<GLframework::Mesh> screenQuad{ nullptr };
		GLframework::FrameRenderTargets frameRenderTargets{};
		std::shared_ptr<GLframework::Bloom> bloom{ nullptr };
		std::shared_ptr<GLframework::GrassInstanceMaterial> grassMaterial{ nullptr };
		std::shared_ptr<GLframework::Mesh> skyBoxMesh{ nullptr };
		std::shared_ptr<GLframework::Mesh> movePlane{ nullptr };
		std::shared_ptr<GLframework::Mesh> textD{ nullptr };
		std::shared_ptr<GLframework::ScreenMaterial> screenMaterial{ nullptr };
		std::shared_ptr<GLframework::PhongCSMShadowMaterial> csmShadowMaterial{ nullptr };
		GLframework::PostProcessPass postProcessPass{};
		glm::vec3 clearColor{};
	};
}
