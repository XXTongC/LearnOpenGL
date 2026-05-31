#include "LegacyExperimentRunner.h"

#include <cstdlib>

#include "../../application/assimpInstanceLoader.h"
#include "../../application/assimpLoader.h"
#include "../../core.h"
#include "../../framework/geometry.h"
#include "../../framework/object.h"
#include "../../framework/scene.h"
#include "../../framework/texture.h"
#include "../../light/directionalLight.h"
#include "../../light/pointLight.h"
#include "../../materials/cubeSphereMaterial.h"
#include "../../materials/grassInstanceMaterial/grassInstanceMaterial.h"
#include "../../materials/phongCSMShadowMaterial/phongCSMShadowMaterial.h"
#include "../../materials/phongEnvSphereMaterial.h"
#include "../../materials/phongInstanceMaterial.h"
#include "../../materials/phongMaterial.h"
#include "../../mesh/instancedMesh.h"
#include "../../mesh/mesh.h"
#include "../../renderer/renderer.h"
#include "../../tools/tools.h"

namespace GL_EXPERIMENTS
{
	struct LegacyExperimentRunner::Impl
	{
		struct SolarSystemState
		{
			bool enabled{ false };
			float speed{ 0.01f };
			std::shared_ptr<GLframework::Object> roundForEarth{ nullptr };
			std::shared_ptr<GLframework::Object> roundForVenus{ nullptr };
			std::shared_ptr<GLframework::Object> roundForUranus{ nullptr };
			std::shared_ptr<GLframework::Object> roundForSaturn{ nullptr };
			std::shared_ptr<GLframework::Object> roundForNeptune{ nullptr };
			std::shared_ptr<GLframework::Object> roundForJupiter{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMars{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMercury{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMoon{ nullptr };
		};

		struct OrbitingPointLightState
		{
			bool enabled{ false };
			std::size_t lightIndex{ 0 };
			float radius{ 3.0f };
			float height{ 3.0f };
		};

		SolarSystemState solarSystem{};
		OrbitingPointLightState orbitingPointLight{};
		bool grassFieldEnabled{ false };
		bool environmentSphereEnabled{ false };
		bool csmPlaneEnabled{ false };
		bool backpackEnabled{ false };
		bool shadowPreviewEnabled{ false };
	};

	LegacyExperimentRunner::LegacyExperimentRunner()
		: mImpl(std::make_unique<Impl>())
	{
	}

	LegacyExperimentRunner::~LegacyExperimentRunner() = default;
	LegacyExperimentRunner::LegacyExperimentRunner(LegacyExperimentRunner&&) noexcept = default;
	LegacyExperimentRunner& LegacyExperimentRunner::operator=(LegacyExperimentRunner&&) noexcept = default;

	void LegacyExperimentRunner::enableSolarSystem(RuntimeContext& context)
	{
		auto& solarSystem = mImpl->solarSystem;
		if (solarSystem.enabled) return;

		solarSystem.enabled = true;
		solarSystem.roundForEarth = std::make_shared<GLframework::Object>();
		solarSystem.roundForVenus = std::make_shared<GLframework::Object>();
		solarSystem.roundForUranus = std::make_shared<GLframework::Object>();
		solarSystem.roundForSaturn = std::make_shared<GLframework::Object>();
		solarSystem.roundForNeptune = std::make_shared<GLframework::Object>();
		solarSystem.roundForJupiter = std::make_shared<GLframework::Object>();
		solarSystem.roundForMars = std::make_shared<GLframework::Object>();
		solarSystem.roundForMercury = std::make_shared<GLframework::Object>();
		solarSystem.roundForMoon = std::make_shared<GLframework::Object>();

		const float distanceEarth = 10.0f;
		const float sizeOfEarth = 1.0f;

		auto sphereGeo = GLframework::Geometry::createSphere(
			context.renderer->getShader(GLframework::MaterialType::PhongMaterial),
			0.3f,
			1000,
			1000
		);

		auto createPlanet = [&](const char* texturePath, float scaleValue, glm::vec3 position) {
			auto mat = std::make_shared<GLframework::PhongMaterial>();
			mat->mDiffuse = std::make_shared<GLframework::Texture>(texturePath, 0);
			auto mesh = std::make_shared<GLframework::Mesh>(sphereGeo, mat);
			mesh->setScale(glm::vec3(scaleValue * sizeOfEarth));
			mesh->setPosition(position);
			return mesh;
		};

		auto moonSphere = createPlanet("Texture/solar system/moon1k.jpg", 0.27f, { 1.1f, 0.0f, 0.0f });
		auto sunSphere = createPlanet("Texture/solar system/2k_sun.jpg", 10.00f, { 0.0f, 0.0f, 0.0f });
		auto venusSphere = createPlanet("Texture/solar system/2k_venus_surface.jpg", 0.94f, { 0.72f * distanceEarth, 0.0f, 0.0f });
		auto uranusSphere = createPlanet("Texture/solar system/2k_uranus.jpg", 4.00f, { 19.19f * distanceEarth, 0.0f, 0.0f });
		auto saturnSphere = createPlanet("Texture/solar system/2k_saturn.jpg", 9.44f, { 9.53f * distanceEarth, 0.0f, 0.0f });
		auto neptuneSphere = createPlanet("Texture/solar system/2k_neptune.jpg", 3.88f, { 30.06f * distanceEarth, 0.0f, 0.0f });
		auto jupiterSphere = createPlanet("Texture/solar system/2k_jupiter.jpg", 11.20f, { 5.20f * distanceEarth, 0.0f, 0.0f });
		auto marsSphere = createPlanet("Texture/solar system/2k_mars.jpg", 1.0f, { 1.52f * distanceEarth, 0.0f, 0.0f });
		auto earthSphere = createPlanet("Texture/solar system/2k_earth_daymap.jpg", 1.0f, { distanceEarth, 0.0f, 0.0f });
		auto earthNightSphere = createPlanet("Texture/solar system/2k_earth_nightmap.jpg", 1.0f, { distanceEarth + 0.001f, 0.0f, 0.0f });
		auto mercurySphere = createPlanet("Texture/solar system/2k_mercury.jpg", 0.38f, { 0.38f * distanceEarth, 0.0f, 0.0f });

		solarSystem.roundForVenus->addChild(venusSphere);
		solarSystem.roundForUranus->addChild(uranusSphere);
		solarSystem.roundForSaturn->addChild(saturnSphere);
		solarSystem.roundForNeptune->addChild(neptuneSphere);
		solarSystem.roundForJupiter->addChild(jupiterSphere);
		solarSystem.roundForMars->addChild(marsSphere);
		solarSystem.roundForMercury->addChild(mercurySphere);

		solarSystem.roundForEarth->addChild(earthSphere);
		solarSystem.roundForEarth->addChild(earthNightSphere);
		solarSystem.roundForEarth->addChild(solarSystem.roundForMoon);
		solarSystem.roundForMoon->setPosition({ distanceEarth, 0.0f, 0.0f });
		solarSystem.roundForMoon->addChild(moonSphere);

		context.sceneOffScreen->addChild(solarSystem.roundForVenus);
		context.sceneOffScreen->addChild(solarSystem.roundForUranus);
		context.sceneOffScreen->addChild(solarSystem.roundForSaturn);
		context.sceneOffScreen->addChild(solarSystem.roundForNeptune);
		context.sceneOffScreen->addChild(solarSystem.roundForJupiter);
		context.sceneOffScreen->addChild(solarSystem.roundForMars);
		context.sceneOffScreen->addChild(solarSystem.roundForEarth);
		context.sceneOffScreen->addChild(solarSystem.roundForMercury);
		context.sceneOffScreen->addChild(sunSphere);
	}

	void LegacyExperimentRunner::enableGrassField(RuntimeContext& context, int rowCount, int columnCount)
	{
		if (mImpl->grassFieldEnabled) return;

		mImpl->grassFieldEnabled = true;
		context.grassMaterial = std::make_shared<GLframework::GrassInstanceMaterial>();
		context.grassMaterial->mDiffuse = std::make_shared<GLframework::Texture>("fbx/textures/GRASS.PNG", 0);
		context.grassMaterial->mOpacityMask = std::make_shared<GLframework::Texture>("fbx/textures/grassMask.png", 2);
		context.grassMaterial->mCloudMask = std::make_shared<GLframework::Texture>("Texture/CLOUD.PNG", 3);

		auto house = GL_APPLICATION::AssimpLoader::load("fbx/house.fbx", context.renderer);
		house->setScale(glm::vec3(0.5f));
		house->setPosition(glm::vec3(rowCount * 0.2f / 2.0f, 0.4f, columnCount * 0.2f / 2.0f));
		context.sceneOffScreen->addChild(house);

		auto grassModel = GL_APPLICATION::AssimpInstanceLoader::load("fbx/grassNew.obj", context.renderer, rowCount * columnCount);
		std::srand(static_cast<unsigned int>(glfwGetTime()));

		for (int i = 0; i < rowCount; ++i)
		{
			for (int j = 0; j < columnCount; ++j)
			{
				auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.2f * i, 0.0f, 0.2f * j));
				auto rotate = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(std::rand() % 90)), glm::vec3(0.0f, 1.0f, 0.0f));
				auto transform = translate * rotate;
				GL_APPLICATION::AssimpInstanceLoader::setInstanceMatrix(grassModel, j + i * columnCount, transform);
			}
		}

		GL_APPLICATION::AssimpInstanceLoader::updateInstanceMatrix(grassModel);
		GL_APPLICATION::AssimpInstanceLoader::setInstanceMaterial(grassModel, context.grassMaterial);
		context.sceneOffScreen->addChild(grassModel);
	}

	void LegacyExperimentRunner::enableEnvironmentSphere(RuntimeContext& context)
	{
		if (mImpl->environmentSphereEnabled) return;
		if (!context.skyBoxMesh) return;

		auto skyBoxMaterial = std::dynamic_pointer_cast<GLframework::CubeSphereMaterial>(context.skyBoxMesh->getMaterial());
		if (!skyBoxMaterial || !skyBoxMaterial->mDiffuse) return;

		mImpl->environmentSphereEnabled = true;

		auto earthMat = std::make_shared<GLframework::PhongEnvSphereMaterial>();
		earthMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_daymap.jpg", 0);
		earthMat->mEnv = skyBoxMaterial->mDiffuse;

		auto earthGeo = GLframework::Geometry::createSphere(
			context.renderer->getShader(GLframework::MaterialType::PhongEnvSphereMaterial),
			1.0f
		);
		auto earthMesh = std::make_shared<GLframework::Mesh>(earthGeo, earthMat);
		context.sceneOffScreen->addChild(earthMesh);

		auto earthInstancedMat = std::make_shared<GLframework::PhongInstanceMaterial>();
		earthInstancedMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/solar system/2k_earth_daymap.jpg", 0);
		auto earthInstancedGeo = GLframework::Geometry::createSphere(
			context.renderer->getShader(earthInstancedMat->getMaterialType()),
			1.0f
		);
		auto earthInstanced = std::make_shared<GLframework::InstancedMesh>(earthInstancedGeo, earthInstancedMat, 2);
		earthInstanced->mInstanceMatrices[0] = glm::mat4(1.0f);
		earthInstanced->mInstanceMatrices[1] = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
		earthInstanced->updateMatrices();
		earthInstanced->setPosition({ 2.0f,0.0f,0.0f });
		context.sceneOffScreen->addChild(earthInstanced);
	}

	void LegacyExperimentRunner::enableCsmPlane(RuntimeContext& context)
	{
		if (mImpl->csmPlaneEnabled) return;

		mImpl->csmPlaneEnabled = true;
		context.csmShadowMaterial = std::make_shared<GLframework::PhongCSMShadowMaterial>();
		context.csmShadowMaterial->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png", 0, GL_SRGB_ALPHA);

		auto boxGeo = GLframework::Geometry::createPlane(
			context.renderer->getShader(context.csmShadowMaterial->getMaterialType()),
			1.0f,
			1.0f
		);
		context.movePlane = std::make_shared<GLframework::Mesh>(boxGeo, context.csmShadowMaterial);
		context.movePlane->setPosition({ 0.0f,0.0f,0.0f });
		context.movePlane->rotateX(-90);
		context.sceneOffScreen->addChild(context.movePlane);

		auto parallaxTestMaterial = std::make_shared<GLframework::PhongCSMShadowMaterial>();
		parallaxTestMaterial->mDiffuse = std::make_shared<GLframework::Texture>("Texture/parallax/bricks.jpg", 0, GL_SRGB_ALPHA);
		auto planeGeo = GLframework::Geometry::createPlane(
			context.renderer->getShader(parallaxTestMaterial->getMaterialType()),
			10.0f,
			10.0f
		);
		auto planeMesh = std::make_shared<GLframework::Mesh>(planeGeo, parallaxTestMaterial);
		planeMesh->rotateX(-90);
		context.sceneOffScreen->addChild(planeMesh);
	}

	void LegacyExperimentRunner::enableBackpackModel(RuntimeContext& context)
	{
		if (mImpl->backpackEnabled) return;

		mImpl->backpackEnabled = true;
		auto backpack = GL_APPLICATION::AssimpLoader::load("fbx/bag/backpack.obj", context.renderer);
		backpack->setScale(glm::vec3(1.0f));
		GLframework::Tools::setModelBlend(backpack, true, 0.5f);
		context.sceneOffScreen->addChild(backpack);
	}

	void LegacyExperimentRunner::enableShadowPreview(RuntimeContext& context)
	{
		if (mImpl->shadowPreviewEnabled) return;
		if (!context.dirLight || !context.dirLight->getShadow()) return;

		mImpl->shadowPreviewEnabled = true;

		auto previewMaterial = std::make_shared<GLframework::PhongMaterial>();
		context.dirLight->getShadow()->mRenderTarget->getDepthAttachment()->setUnit(2);
		previewMaterial->mDiffuse = context.dirLight->getShadow()->mRenderTarget->getDepthAttachment();

		auto previewGeo = GLframework::Geometry::createPlane(
			context.renderer->getShader(previewMaterial->getMaterialType()),
			2.0f,
			2.0f
		);
		auto previewMesh = std::make_shared<GLframework::Mesh>(previewGeo, previewMaterial);
		previewMesh->setPosition({ 3.0f,1.0f,0.0f });
		context.sceneOffScreen->addChild(previewMesh);
	}

	void LegacyExperimentRunner::enableOrbitingPointLight(std::size_t lightIndex, float radius, float height)
	{
		mImpl->orbitingPointLight.enabled = true;
		mImpl->orbitingPointLight.lightIndex = lightIndex;
		mImpl->orbitingPointLight.radius = radius;
		mImpl->orbitingPointLight.height = height;
	}

	void LegacyExperimentRunner::update(RuntimeContext& context)
	{
		updateSolarSystem();
		updateOrbitingPointLight(context);
	}

	void LegacyExperimentRunner::updateSolarSystem()
	{
		auto& solarSystem = mImpl->solarSystem;
		if (!solarSystem.enabled) return;

		solarSystem.roundForVenus->rotateY(1.6022f * solarSystem.speed);
		for (auto& child : solarSystem.roundForVenus->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForUranus->rotateY(0.0117f * solarSystem.speed);
		for (auto& child : solarSystem.roundForUranus->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForEarth->rotateY(0.9863f * solarSystem.speed);
		for (auto& child : solarSystem.roundForEarth->getChildren())
		{
			if (child->getType() == GLframework::ObjectType::Mesh) child->rotateY(solarSystem.speed * 10.0f);
		}
		solarSystem.roundForMoon->rotateY(5.0f * solarSystem.speed);

		solarSystem.roundForJupiter->rotateY(0.08316f * solarSystem.speed);
		for (auto& child : solarSystem.roundForJupiter->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForMars->rotateY(0.5240f * solarSystem.speed);
		for (auto& child : solarSystem.roundForMars->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForSaturn->rotateY(0.0335f * solarSystem.speed);
		for (auto& child : solarSystem.roundForSaturn->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForMercury->rotateY(4.0927f * solarSystem.speed);
		for (auto& child : solarSystem.roundForMercury->getChildren()) child->rotateY(solarSystem.speed * 10.0f);

		solarSystem.roundForNeptune->rotateY(0.0059f * solarSystem.speed);
		for (auto& child : solarSystem.roundForNeptune->getChildren()) child->rotateY(solarSystem.speed * 10.0f);
	}

	void LegacyExperimentRunner::updateOrbitingPointLight(RuntimeContext& context)
	{
		const auto& orbitingPointLight = mImpl->orbitingPointLight;
		if (!orbitingPointLight.enabled) return;
		if (orbitingPointLight.lightIndex >= context.pointLights.size()) return;

		const double time = glfwGetTime();
		const float x = static_cast<float>(orbitingPointLight.radius * glm::sin(time));
		const float z = static_cast<float>(orbitingPointLight.radius * glm::cos(time));
		context.pointLights[orbitingPointLight.lightIndex]->setPosition({ x, orbitingPointLight.height, z });
	}
}
