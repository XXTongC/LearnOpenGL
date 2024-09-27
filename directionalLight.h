#pragma once
#include "light.h"

namespace GLframework
{
	class DirectionalLight : public Light
	{
	public:
		DirectionalLight()
			:Light()
		{}
		~DirectionalLight(){}
		void setDirection(glm::vec3 value);
		glm::vec3 getDirection() const;
	private:
		glm::vec3 mDirection{ -1.0f };
	};
}
