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
	};
}
