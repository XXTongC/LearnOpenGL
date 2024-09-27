#pragma once
#include "core.h"

namespace GLframework
{
	class Light
	{
	public:
		Light() {}
		~Light()
		{
			std::cout << "light destroy" << std::endl;
		};
		glm::vec3 getColor() const;
		float getSpecularIntensity() const;
		void setColor(glm::vec3 value);
		void setSpecularIntensity(float value);
	private:
		glm::vec3 mColor{ 1.0f };
		float mSpecularIntensity{ 1.0f };
	};
}
