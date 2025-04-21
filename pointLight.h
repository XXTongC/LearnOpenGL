#pragma once
#include "core.h"
#include "light.h"
#include "object.h"
namespace GLframework
{
	class PointLight : public Light
	{
	public:
		PointLight();
		~PointLight() {}
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
