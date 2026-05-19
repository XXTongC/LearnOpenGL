#pragma once

#include <memory>
#include <vector>

namespace GLframework
{
	class DirectionalLight;
	class Mesh;
	class PointLight;
	class ScreenMaterial;
}

namespace GL_EDITOR
{
	struct DebugControllerContext
	{
		std::shared_ptr<GLframework::DirectionalLight> directionalLight{ nullptr };
		std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		std::shared_ptr<GLframework::Mesh> textObject{ nullptr };
		std::shared_ptr<GLframework::ScreenMaterial> screenMaterial{ nullptr };
		float* orbitAngle{ nullptr };
	};

	void drawDebugControllerPanel(const DebugControllerContext& context);
}
