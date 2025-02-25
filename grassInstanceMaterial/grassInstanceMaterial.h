#pragma once

#include <memory>
#include "../material.h"
#include "../texture.h"

namespace GLframework
{
	class Texture;

	class GrassInstanceMaterial : public Material
	{
	public:
		GrassInstanceMaterial();
		GrassInstanceMaterial(std::shared_ptr<Texture> diffuse, float shiness)
			:mDiffuse(std::move(diffuse)), mShiness(shiness)
		{
			setColorBlendState(true);
			setDepthWrite(false);
			setMaterialType(MaterialType::GrassInstanceMaterial);
		}
		~GrassInstanceMaterial();
		void setUVScale(float value);
		void setBrightness(float value);
		void setWindScale(float value);
		void setWindDirection(glm::vec3 value);
		void setPhaseScale(float value);
		void setCloudLerp(float value);
		void setCloudUVScale(float value);
		void setCloudSpeed(float value);
		void seCloudWhiteColor(glm::vec3 value);
		void setCloudBlackColor(glm::vec3 value);

		float getUVScale() const;
		float getBrightness() const;
		float getWindScale() const;
		float getCloudLerp() const;
		float getPhaseScale() const;
		float getCloudSpeed() const;
		float getCloudUVScale() const;
		glm::vec3 getWindDirection() const;
		glm::vec3 getCloudWhiteColor() const;
		glm::vec3 getCloudBlackColor() const;

		float* Control_UVScale();
		float* Control_Brightness();
		float* Control_WindScale();
		float* Control_CloudUVScale();
		float* Control_CloudSpeed();
		float* Control_CloudLerp();
		float* Control_PhaseScale();
		glm::vec3* Control_WindDirection();
		glm::vec3* Control_CloudWhiteColor();
		glm::vec3* Control_CloudBlackColor();


	public:
		std::shared_ptr<Texture> mDiffuse{ nullptr };
		std::shared_ptr<Texture> mSpecularMask{ nullptr };
		std::shared_ptr<Texture> mOpacityMask{ nullptr };
		std::shared_ptr<Texture> mCloudMask{ nullptr };
		float mShiness{ 10.0f };
	private:
		//grass texture attribute
		float mUVScale{ 1.0f };
		float mBrightness{ 1.0f };
		float mWindScale{ 0.0f };
		float mPhaseScale{1.0f};
		glm::vec3 mWindDirection{ 1.0f,0.0f,0.0f };

		//could texture attribute
		glm::vec3 mCloudWhiteColor{ 1.0f,0.0f,0.0f };
		glm::vec3 mCloudBlackColor{ 0.0f,0.0f,1.0f };
		float mCloudUVScale{ 1.0f };
		float mCloudSpeed{ 0.1f };
		float mCloudLerp{ 0.2f };

	};
}
