#pragma once
#include "core.h"
#include "object.h"
namespace GLframework
{
	class Light:public Object
	{
	public:
		Light()
			:Object()
		{}
		~Light()
		{
			std::cout << "light destroy" << std::endl;
		}
		glm::vec3 getColor() const;
		float getSpecularIntensity() const;
		float getIntensity() const;
		float* Control_Intensity();

		void setIntensity(float value);
		void setColor(glm::vec3 value);
		void setSpecularIntensity(float value);
	private:
		glm::vec3 mColor{ 1.0f };
		float mSpecularIntensity{ 1.0f };
		float mIntensity{ 1.0f };
	};
}
