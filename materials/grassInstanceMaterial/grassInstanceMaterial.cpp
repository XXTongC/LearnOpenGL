#include "grassInstanceMaterial.h"
#include "../../tools/inspector/MaterialInspector.h"

using namespace GLframework;

GrassInstanceMaterial::GrassInstanceMaterial()
{
	setColorBlendState(true);
	mSpecularMask = std::make_shared<GLframework::Texture>("Texture/defaultMaskSpecular.png", 1);
	setMaterialType(MaterialType::GrassInstanceMaterial);
}

GrassInstanceMaterial::~GrassInstanceMaterial()
{
}

void GrassInstanceMaterial::visitEditableProperties(GL_EDITOR::PropertyBuilder& builder)
{
	Material::visitEditableProperties(builder);

	builder.addSection("Surface");
	builder.addFloat("Shininess", &mShiness, 0.0f, 256.0f, "%.1f");
	builder.addFloat("UV Scale", Control_UVScale(), 0.0f, 100.0f);
	builder.addFloat("Brightness", Control_Brightness(), 0.0f, 8.0f);
	builder.addText("Diffuse", [this]() { return GL_EDITOR::describeTexture(mDiffuse); });
	builder.addText("Opacity Mask", [this]() { return GL_EDITOR::describeTexture(mOpacityMask); });
	builder.addText("Cloud Mask", [this]() { return GL_EDITOR::describeTexture(mCloudMask); });

	builder.addSection("Wind");
	builder.addFloat("Wind Scale", Control_WindScale(), -0.12f, 0.12f, "%.4f");
	builder.addFloat("Phase Scale", Control_PhaseScale(), 0.0f, 10.0f);
	builder.addVec3(
		"Wind Direction",
		[this]() { return *Control_WindDirection(); },
		[this](glm::vec3 value) { setWindDirection(value); }
	);

	builder.addSection("Cloud");
	builder.addFloat("Cloud Lerp", Control_CloudLerp(), 0.0f, 1.0f);
	builder.addFloat("Cloud UV Scale", Control_CloudUVScale(), 0.0f, 100.0f);
	builder.addFloat("Cloud Speed", Control_CloudSpeed(), 0.0f, 3.0f);
	builder.addColor3(
		"Cloud White Color",
		[this]() { return *Control_CloudWhiteColor(); },
		[this](glm::vec3 value) { seCloudWhiteColor(value); }
	);
	builder.addColor3(
		"Cloud Black Color",
		[this]() { return *Control_CloudBlackColor(); },
		[this](glm::vec3 value) { setCloudBlackColor(value); }
	);
}

float GrassInstanceMaterial::getBrightness() const
{
	return mBrightness;
}

float GrassInstanceMaterial::getUVScale() const
{
	return mUVScale;
}

glm::vec3 GrassInstanceMaterial::getWindDirection() const
{
	return mWindDirection;
}

float GrassInstanceMaterial::getWindScale() const
{
	return mWindScale;
}

void GrassInstanceMaterial::setBrightness(float value)
{
	mBrightness = value;
}

void GrassInstanceMaterial::setUVScale(float value)
{
	mUVScale = value;
}

void GrassInstanceMaterial::setWindDirection(glm::vec3 value)
{
	mWindDirection = glm::normalize(value);
}

void GrassInstanceMaterial::setWindScale(float value)
{
	mWindScale = value;
}

float* GrassInstanceMaterial::Control_Brightness()
{
	return &mBrightness;
}

float* GrassInstanceMaterial::Control_UVScale()
{
	return &mUVScale;
}

glm::vec3* GrassInstanceMaterial::Control_WindDirection()
{
	return &mWindDirection;
}

float* GrassInstanceMaterial::Control_WindScale()
{
	return &mWindScale;
}

float* GrassInstanceMaterial::Control_PhaseScale()
{
	return &mPhaseScale;
}

float GrassInstanceMaterial::getPhaseScale() const
{
	return mPhaseScale;
}

void GrassInstanceMaterial::setPhaseScale(float value)
{
	mPhaseScale = value;
}

glm::vec3* GrassInstanceMaterial::Control_CloudBlackColor()
{
	return &mCloudBlackColor;
}

glm::vec3* GrassInstanceMaterial::Control_CloudWhiteColor()
{
	return &mCloudWhiteColor;
}

glm::vec3 GrassInstanceMaterial::getCloudBlackColor() const
{
	return mCloudBlackColor;
}

glm::vec3 GrassInstanceMaterial::getCloudWhiteColor() const
{
	return mCloudWhiteColor;
}

void GrassInstanceMaterial::seCloudWhiteColor(glm::vec3 value)
{
	mCloudWhiteColor = value;
}

void GrassInstanceMaterial::setCloudBlackColor(glm::vec3 value)
{
	mCloudBlackColor = value;
}

float* GrassInstanceMaterial::Control_CloudUVScale()
{
	return &mCloudUVScale;
}

void GrassInstanceMaterial::setCloudUVScale(float value)
{
	mCloudUVScale = value;
}

float GrassInstanceMaterial::getCloudUVScale() const
{
	return mCloudUVScale;
}

float* GrassInstanceMaterial::Control_CloudSpeed()
{
	return &mCloudSpeed;
}

void GrassInstanceMaterial::setCloudSpeed(float value)
{
	mCloudSpeed = value;
}

float GrassInstanceMaterial::getCloudSpeed() const
{
	return mCloudSpeed;
}

float* GrassInstanceMaterial::Control_CloudLerp()
{
	return &mCloudLerp;
}

void GrassInstanceMaterial::setCloudLerp(float value)
{
	mCloudLerp = value;
}

float GrassInstanceMaterial::getCloudLerp() const
{
	return mCloudLerp;
}
