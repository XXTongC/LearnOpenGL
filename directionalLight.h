#pragma once
#include "light.h"
#include "light/shadow/directionalLightShadow/directionalLightShadow.h"
#include "light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
namespace GLframework
{
	class DirectionalLight : public Light
	{
	public:
		DirectionalLight()
			:Light()
		{
			setShadow(std::make_shared<DirectionalLightCSMShadow>());
		}
		~DirectionalLight() override{}
	};
}
