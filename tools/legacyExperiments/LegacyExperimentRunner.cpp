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
	void LegacyExperimentRunner::enableSolarSystem(RuntimeContext& context)
	{
		if (mSolarSystem.enabled) return;

		mSolarSystem.enabled = true;
		mSolarSystem.roundForEarth = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForVenus = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForUranus = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForSaturn = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForNeptune = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForJupiter = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForMars = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForMercury = std::make_shared<GLframework::Object>();
		mSolarSystem.roundForMoon = std::make_shared<GLframework::Object>();

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

		mSolarSystem.roundForVenus->addChild(venusSphere);
		mSolarSystem.roundForUranus->addChild(uranusSphere);
		mSolarSystem.roundForSaturn->addChild(saturnSphere);
		mSolarSystem.roundForNeptune->addChild(neptuneSphere);
		mSolarSystem.roundForJupiter->addChild(jupiterSphere);
		mSolarSystem.roundForMars->addChild(marsSphere);
		mSolarSystem.roundForMercury->addChild(mercurySphere);

		mSolarSystem.roundForEarth->addChild(earthSphere);
		mSolarSystem.roundForEarth->addChild(earthNightSphere);
		mSolarSystem.roundForEarth->addChild(mSolarSystem.roundForMoon);
		mSolarSystem.roundForMoon->setPosition({ distanceEarth, 0.0f, 0.0f });
		mSolarSystem.roundForMoon->addChild(moonSphere);

		context.sceneOffScreen->addChild(mSolarSystem.roundForVenus);
		context.sceneOffScreen->addChild(mSolarSystem.roundForUranus);
		context.sceneOffScreen->addChild(mSolarSystem.roundForSaturn);
		context.sceneOffScreen->addChild(mSolarSystem.roundForNeptune);
		context.sceneOffScreen->addChild(mSolarSystem.roundForJupiter);
		context.sceneOffScreen->addChild(mSolarSystem.roundForMars);
		context.sceneOffScreen->addChild(mSolarSystem.roundForEarth);
		context.sceneOffScreen->addChild(mSolarSystem.roundForMercury);
		context.sceneOffScreen->addChild(sunSphere);
	}

	void LegacyExperimentRunner::enableGrassField(RuntimeContext& context, int rowCount, int columnCount)
	{
		if (mGrassFieldEnabled) return;

		mGrassFieldEnabled = true;
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
		if (mEnvironmentSphereEnabled) return;
		if (!context.skyBoxMesh) return;

		auto skyBoxMaterial = std::dynamic_pointer_cast<GLframework::CubeSphereMaterial>(context.skyBoxMesh->getMaterial());
		if (!skyBoxMaterial || !skyBoxMaterial->mDiffuse) return;

		mEnvironmentSphereEnabled = true;

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
		if (mCsmPlaneEnabled) return;

		mCsmPlaneEnabled = true;
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
		if (mBackpackEnabled) return;

		mBackpackEnabled = true;
		auto backpack = GL_APPLICATION::AssimpLoader::load("fbx/bag/backpack.obj", context.renderer);
		backpack->setScale(glm::vec3(1.0f));
		GLframework::Tools::setModelBlend(backpack, true, 0.5f);
		context.sceneOffScreen->addChild(backpack);
	}

	void LegacyExperimentRunner::enableShadowPreview(RuntimeContext& context)
	{
		if (mShadowPreviewEnabled) return;
		if (!context.dirLight || !context.dirLight->getShadow()) return;

		mShadowPreviewEnabled = true;

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
		mOrbitingPointLight.enabled = true;
		mOrbitingPointLight.lightIndex = lightIndex;
		mOrbitingPointLight.radius = radius;
		mOrbitingPointLight.height = height;
	}

	void LegacyExperimentRunner::update(RuntimeContext& context)
	{
		updateSolarSystem();
		updateOrbitingPointLight(context);
	}

	void LegacyExperimentRunner::updateSolarSystem()
	{
		if (!mSolarSystem.enabled) return;

		mSolarSystem.roundForVenus->rotateY(1.6022f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForVenus->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForUranus->rotateY(0.0117f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForUranus->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForEarth->rotateY(0.9863f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForEarth->getChildren())
		{
			if (child->getType() == GLframework::ObjectType::Mesh) child->rotateY(mSolarSystem.speed * 10.0f);
		}
		mSolarSystem.roundForMoon->rotateY(5.0f * mSolarSystem.speed);

		mSolarSystem.roundForJupiter->rotateY(0.08316f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForJupiter->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForMars->rotateY(0.5240f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForMars->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForSaturn->rotateY(0.0335f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForSaturn->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForMercury->rotateY(4.0927f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForMercury->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);

		mSolarSystem.roundForNeptune->rotateY(0.0059f * mSolarSystem.speed);
		for (auto& child : mSolarSystem.roundForNeptune->getChildren()) child->rotateY(mSolarSystem.speed * 10.0f);
	}

	void LegacyExperimentRunner::updateOrbitingPointLight(RuntimeContext& context)
	{
		if (!mOrbitingPointLight.enabled) return;
		if (mOrbitingPointLight.lightIndex >= context.pointLights.size()) return;

		const double time = glfwGetTime();
		const float x = static_cast<float>(mOrbitingPointLight.radius * glm::sin(time));
		const float z = static_cast<float>(mOrbitingPointLight.radius * glm::cos(time));
		context.pointLights[mOrbitingPointLight.lightIndex]->setPosition({ x, mOrbitingPointLight.height, z });
	}
}
