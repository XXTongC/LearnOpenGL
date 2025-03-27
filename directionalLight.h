#pragma once
#include "light.h"
#include "light/shadow/directionalLightShadow/directionalLightShadow.h"
namespace GLframework
{
	class DirectionalLight : public Light
	{
	public:
		DirectionalLight()
			:Light()
		{
			setShadow(std::make_shared<DirectionalLightShadow>());
		}
		~DirectionalLight() override{}
	};
}
