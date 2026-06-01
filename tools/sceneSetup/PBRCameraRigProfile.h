#pragma once

#include "../../third_party/glm/glm.hpp"

class Camera;

namespace GL_SCENE
{
	struct PBRCameraRigProfile
	{
		glm::vec3 position{ 0.0f, 0.0f, 5.0f };
		glm::vec3 up{ 0.0f, 1.0f, 0.0f };
		glm::vec3 right{ 1.0f, 0.0f, 0.0f };
		float fovy{ 60.0f };
		float nearPlane{ 0.1f };
		float farPlane{ 1000.0f };

		void applyTo(Camera* camera) const;
		void copyFrom(const Camera* camera);
	};
}
