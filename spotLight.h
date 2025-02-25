#pragma once
#include "light.h"
#include "object.h"

namespace GLframework
{
	class SpotLight : public Light
	{
	public:
		SpotLight(glm::vec3 targetDirection = glm::vec3{ -1.0f },float innerangle = 1.0f,float outangle = 2.0f)
			:Light(),mTargetDirection(targetDirection),mInnerAngle(innerangle),mOutAngle(outangle)
		{}
		~SpotLight(){}
		void setTargetDirection(glm::vec3 value);
		//void setVisibleAngle(float value);
		glm::vec3 getTargetDirection() const;
		void setInnerAngle(float value);
		void setOutAngle(float value);
		float getInnerAngle() const;
		float getOutAngle() const;

	private:
		glm::vec3 mTargetDirection;
		//float mVisibleAngle;
		float mInnerAngle;
		float mOutAngle;
	};

}
