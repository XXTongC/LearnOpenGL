#pragma once
#include "../shadow.h"


namespace GLframework
{
	class DirectionalLightShadow : public Shadow
	{
	public:
		DirectionalLightShadow();
		~DirectionalLightShadow() override;
		virtual void setRenderTargetSize(int width, int height) override;
		glm::mat4 getLightMatrix(glm::mat4 lightModelMatrix);

	};
}
