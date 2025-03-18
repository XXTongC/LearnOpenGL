#pragma once
#include "light.h"
#include "object.h"

namespace GLframework
{
	class SpotLight : public Light
	{
	public:
		SpotLight(float innerangle = 1.0f,float outangle = 2.0f)
			:Light(),mInnerAngle(innerangle),mOutAngle(outangle)
		{}
		~SpotLight(){}

		void setInnerAngle(float value);
		void setOutAngle(float value);
		float getInnerAngle() const;
		float getOutAngle() const;

	private:
		float mInnerAngle;
		float mOutAngle;
	};

}
