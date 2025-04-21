#pragma once
#include "../shadow.h"
#include "../core.h"

namespace GLframework
{
	class DirectionalLightCSMShadow : public Shadow
	{
	public:
		DirectionalLightCSMShadow();
		~DirectionalLightCSMShadow() override;
		void setRenderTargetSize(int width, int height) override;
		void generateCascadeLayers(std::vector<float>& layers,float near,float far);
		void setLayerCount(int value);
		int getLayerCount() const;

		//Pass in the current player camera, light source direction, and near and far values
		glm::mat4 getLightMatrix(Camera* camera,glm::vec3 lightDir,float near,float far);
		std::vector<glm::mat4> getLightMatrix(Camera* camera, glm::vec3 lightDir, const std::vector<float>& clips);
	private:
		int mLayerCount = 5;

	};
}
