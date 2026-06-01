#include "SceneObjectInspector.h"

#include "../../camera/camera.h"
#include "../../camera/orthographiccamera.h"
#include "../../camera/perspectivecamera.h"
#include "../../framebuffer/framebuffer.h"
#include "../../light/light.h"
#include "../../light/pointLight.h"
#include "../../light/shadow/directionalLightCSMShadow/directionalLightCSMShadow.h"
#include "../../light/shadow/directionalLightShadow/directionalLightShadow.h"
#include "../../light/shadow/pointLightShadow/pointLightShadow.h"
#include "../../light/shadow/shadow.h"
#include "../../light/spotLight.h"
#include "../../framework/object.h"

namespace
{
	int clampPositiveSize(int value)
	{
		return value < 1 ? 1 : value;
	}

	std::string objectTypeNameFromEnum(GLframework::ObjectType type)
	{
		switch (type)
		{
		case GLframework::ObjectType::Object: return "Object";
		case GLframework::ObjectType::Mesh: return "Mesh";
		case GLframework::ObjectType::Scene: return "Scene";
		case GLframework::ObjectType::InstancedMesh: return "InstancedMesh";
		case GLframework::ObjectType::Light: return "Light";
		default: return "Unknown";
		}
	}
}

namespace GL_EDITOR
{
	std::string getCameraTypeName(Camera* camera)
	{
		if (dynamic_cast<PerspectiveCamera*>(camera)) return "PerspectiveCamera";
		if (dynamic_cast<OrthographicCamera*>(camera)) return "OrthographicCamera";
		return "Camera";
	}

	std::string getObjectDisplayName(const std::shared_ptr<GLframework::Object>& object)
	{
		if (!object) return "Null";

		const std::string explicitName = object->getName();
		if (!explicitName.empty()) return explicitName;

		return objectTypeNameFromEnum(object->getType());
	}

	std::string getObjectTypeName(const std::shared_ptr<GLframework::Object>& object)
	{
		if (!object) return "Null";
		return objectTypeNameFromEnum(object->getType());
	}

	std::string getShadowTypeName(const std::shared_ptr<GLframework::Shadow>& shadow)
	{
		if (!shadow) return "Shadow";
		if (std::dynamic_pointer_cast<GLframework::DirectionalLightCSMShadow>(shadow)) return "DirectionalLightCSMShadow";
		if (std::dynamic_pointer_cast<GLframework::DirectionalLightShadow>(shadow)) return "DirectionalLightShadow";
		if (std::dynamic_pointer_cast<GLframework::PointLightShadow>(shadow)) return "PointLightShadow";
		return "Shadow";
	}

	PropertyBuilder buildCameraPropertySchema(Camera* camera)
	{
		PropertyBuilder builder{};
		if (!camera)
		{
			return builder;
		}

		builder.addSection("Camera");
		builder.addVec3("Position", &camera->mPosition);
		builder.addVec3("Up", &camera->mUp);
		builder.addVec3("Right", &camera->mRight);
		builder.addInputFloat("Near", &camera->mNear);
		builder.addInputFloat("Far", &camera->mFar);

		if (auto* perspective = dynamic_cast<PerspectiveCamera*>(camera))
		{
			builder.addSection("Perspective");
			builder.addFloat(
				"Fovy",
				[perspective]() { return perspective->getFovy(); },
				[perspective](float value) { perspective->setFovy(value); },
				1.0f,
				179.0f
			);
			builder.addInputFloat(
				"Aspect",
				[perspective]() { return perspective->getAspect(); },
				[perspective](float value) { perspective->setAspect(value); }
			);
		}

		if (auto* orthographic = dynamic_cast<OrthographicCamera*>(camera))
		{
			builder.addSection("Orthographic");
			builder.addInputFloat("Left", &orthographic->mL);
			builder.addInputFloat("Right", &orthographic->mR);
			builder.addInputFloat("Top", &orthographic->mT);
			builder.addInputFloat("Bottom", &orthographic->mB);
		}

		return builder;
	}

	PropertyBuilder buildLightPropertySchema(const std::shared_ptr<GLframework::Light>& light)
	{
		PropertyBuilder builder{};
		if (!light)
		{
			return builder;
		}

		builder.addSection("Light");
		builder.addColor3(
			"Light Color",
			[light]() { return light->getColor(); },
			[light](glm::vec3 value) { light->setColor(value); }
		);
		builder.addFloat(
			"Intensity",
			[light]() { return light->getIntensity(); },
			[light](float value) { light->setIntensity(value); },
			0.0f,
			10.0f
		);
		builder.addFloat(
			"Specular",
			[light]() { return light->getSpecularIntensity(); },
			[light](float value) { light->setSpecularIntensity(value); },
			0.0f,
			10.0f
		);

		if (auto pointLight = std::dynamic_pointer_cast<GLframework::PointLight>(light))
		{
			builder.addSection("Point Light");
			builder.addVec3(
				"Attenuation (k2,k1,k0)",
				[pointLight]()
				{
					return glm::vec3{
						pointLight->getK2(),
						pointLight->getK1(),
						pointLight->getK0()
					};
				},
				[pointLight](glm::vec3 value)
				{
					pointLight->setK(value.x, value.y, value.z);
				}
			);
		}

		if (auto spot = std::dynamic_pointer_cast<GLframework::SpotLight>(light))
		{
			builder.addSection("Spot Light");
			builder.addFloat(
				"Inner Angle",
				[spot]() { return spot->getInnerAngle(); },
				[spot](float value) { spot->setInnerAngle(value); },
				0.0f,
				90.0f
			);
			builder.addFloat(
				"Outer Angle",
				[spot]() { return spot->getOutAngle(); },
				[spot](float value) { spot->setOutAngle(value); },
				0.0f,
				90.0f
			);
		}

		return builder;
	}

	PropertyBuilder buildObjectTransformPropertySchema(const std::shared_ptr<GLframework::Object>& object)
	{
		PropertyBuilder builder{};
		if (!object)
		{
			return builder;
		}

		builder.addSection("Transform");
		builder.addVec3(
			"Position",
			[object]() { return object->getPosition(); },
			[object](glm::vec3 value) { object->setPosition(value); }
		);
		builder.addSliderVec3(
			"Rotation",
			[object]()
			{
				return glm::vec3{
					object->getAngleX(),
					object->getAngleY(),
					object->getAngleZ()
				};
			},
			[object](glm::vec3 value)
			{
				object->setAngleX(value.x);
				object->setAngleY(value.y);
				object->setAngleZ(value.z);
			},
			-360.0f,
			360.0f
		);
		builder.addVec3(
			"Scale",
			[object]() { return object->getScale(); },
			[object](glm::vec3 value) { object->setScale(value); }
		);

		return builder;
	}

	PropertyBuilder buildShadowPropertySchema(const std::shared_ptr<GLframework::Shadow>& shadow)
	{
		PropertyBuilder builder{};
		if (!shadow)
		{
			return builder;
		}

		builder.addSection("Shadow");
		builder.addFloat("Bias", &shadow->mBias, 0.0f, 0.01f, "%.6f");
		builder.addFloat("PCF Radius", &shadow->mPcfRadius, 0.0f, 10.0f, "%.3f");
		builder.addFloat("Disk Tightness", &shadow->mDiskTightness, 0.0f, 4.0f, "%.3f");
		builder.addFloat("Light Size", &shadow->mLightSize, 0.0f, 1.0f, "%.3f");

		if (shadow->mRenderTarget)
		{
			builder.addSection("Shadow Render Target");
			builder.addInputInt(
				"Shadow Width",
				[shadow]()
				{
					return static_cast<int>(shadow->mRenderTarget->getWidth());
				},
				[shadow](int value)
				{
					if (!shadow->mRenderTarget) return;
					shadow->setRenderTargetSize(
						clampPositiveSize(value),
						static_cast<int>(shadow->mRenderTarget->getHeight())
					);
				}
			);
			builder.addInputInt(
				"Shadow Height",
				[shadow]()
				{
					return static_cast<int>(shadow->mRenderTarget->getHeight());
				},
				[shadow](int value)
				{
					if (!shadow->mRenderTarget) return;
					shadow->setRenderTargetSize(
						static_cast<int>(shadow->mRenderTarget->getWidth()),
						clampPositiveSize(value)
					);
				}
			);
		}

		if (auto csmShadow = std::dynamic_pointer_cast<GLframework::DirectionalLightCSMShadow>(shadow))
		{
			builder.addSection("CSM Shadow");
			builder.addInt(
				"Cascade Layers",
				[csmShadow]() { return csmShadow->getLayerCount(); },
				[csmShadow](int value)
				{
					csmShadow->setLayerCount(value);
					if (csmShadow->mRenderTarget)
					{
						csmShadow->setRenderTargetSize(
							static_cast<int>(csmShadow->mRenderTarget->getWidth()),
							static_cast<int>(csmShadow->mRenderTarget->getHeight())
						);
					}
				},
				1,
				8
			);
		}

		if (auto pointShadow = std::dynamic_pointer_cast<GLframework::PointLightShadow>(shadow))
		{
			builder.addSection("Point Shadow");
			builder.addInputInt(
				"Shadow Map Index",
				[pointShadow]() { return pointShadow->getShadowMapIndex(); },
				[pointShadow](int value) { pointShadow->setShadowMapIndex(value); }
			);
			builder.addReadOnlyInt("Max Point Lights", GLframework::PointLightShadow::getMAX_POINT_LIGHT());
		}

		return builder;
	}
}
