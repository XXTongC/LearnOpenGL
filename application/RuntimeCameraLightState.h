#pragma once

#include <memory>
#include <vector>

class Camera;
class CameraControl;

namespace GLframework
{
	class AmbientLight;
	class DirectionalLight;
	class PointLight;
	class SpotLight;
}

namespace GL_RUNTIME
{
	struct RuntimeCameraLightState
	{
		Camera* camera{ nullptr };
		CameraControl* cameracontrol{ nullptr };
		std::shared_ptr<GLframework::AmbientLight> ambientLight{ nullptr };
		std::shared_ptr<GLframework::DirectionalLight> dirLight{ nullptr };
		std::shared_ptr<GLframework::SpotLight> spotLight{ nullptr };
		std::vector<std::shared_ptr<GLframework::PointLight>> pointLights{};
	};
}
