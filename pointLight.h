#pragma once
#include "core.h"
#include "light.h"
#include "object.h"
namespace GLframework
{
	class PointLight : public Light
	{
	public:
		PointLight(float k2=1.0f,float k1=1.0f,float k0=1.0f)
			:Light(),mK2(k2),mK1(k1),mK0(k0)
		{}
		~PointLight() {};
		void setK(float k2, float k1, float k0);
		float getK2() const;
		float getK1() const;
		float getK0() const;

	private:
		float mK2;
		float mK1;
		float mK0;
	};
}
