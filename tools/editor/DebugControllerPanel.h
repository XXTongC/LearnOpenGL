#pragma once

#include <memory>
#include <string>
#include <vector>

namespace GLframework
{
	class DirectionalLight;
	struct EnvironmentProfile;
	class Mesh;
	class PointLight;
	struct PostProcessSettings;
	class Renderer;
}

namespace GL_SCENE
{
	struct PBRPreviewProfile;
}

namespace GL_EDITOR
{
	struct DebugControllerContext
	{
		std::shared_ptr<GLframework::DirectionalLight> directionalLight{ nullptr };
		std::vector<std::shared_ptr<GLframework::PointLight>>* pointLights{ nullptr };
		std::shared_ptr<GLframework::Mesh> textObject{ nullptr };
		GLframework::PostProcessSettings* postProcessSettings{ nullptr };
		const std::string* postProcessSettingsPath{ nullptr };
		std::shared_ptr<GLframework::Renderer> renderer{ nullptr };
		GLframework::EnvironmentProfile* environmentProfile{ nullptr };
		const std::string* environmentProfilePath{ nullptr };
		GL_SCENE::PBRPreviewProfile* pbrPreviewProfile{ nullptr };
		const std::string* pbrPreviewProfilePath{ nullptr };
		float* orbitAngle{ nullptr };
	};

	void drawDebugControllerPanel(const DebugControllerContext& context);
}
