#include "SceneSetup.h"

#include "../../light/shadow/pointLightShadow/pointLightShadow.h"
#include "../../materials/cubeSphereMaterial.h"
#include "../../materials/pbrMaterial/PBRMaterial.h"
#include "../../materials/phongPointShadowMaterial/phongPointShadowMaterial.h"
#include "../../tools/Logger/LogManager.h"

namespace
{
	void prepareRenderResources(GL_SCENE::SetupContext& context)
	{
		context.renderer = std::make_shared<GLframework::Renderer>();
		context.sceneInScreen = std::make_shared<GLframework::Scene>();
		context.sceneOffScreen = std::make_shared<GLframework::Scene>();
		context.sceneInScreen->setName("Screen Scene");
		context.sceneOffScreen->setName("World Scene");

		context.framebufferMultisample = GLframework::Framebuffer::createMultiSampleFbo(context.width, context.height, 4);
		context.framebufferResolve = GLframework::Framebuffer::createHDRFbo(context.width, context.height);

		GLframework::PointLightShadow::initializeSharedDepthTexture(1024, 1024, 2);
	}

	void prepareSkyBox(GL_SCENE::SetupContext& context)
	{
		LogInfo("SkyBox Starting prepare");
		auto skyBoxMat = std::make_shared<GLframework::CubeSphereMaterial>();
		skyBoxMat->mDiffuse = std::make_shared<GLframework::Texture>(context.texturePath, 0);
		auto boxGeo = GLframework::Geometry::createBox(
			context.renderer->getShader(GLframework::MaterialType::CubeSphereMaterial),
			3.0f,
			3.0f,
			3.0f
		);
		context.skyBoxMesh = std::make_shared<GLframework::Mesh>(boxGeo, skyBoxMat);
		context.skyBoxMesh->setName("SkyBox");
		LogInfo("SkyBox Prepared");
	}

	void prepareRoomScene(GL_SCENE::SetupContext& context)
	{
		auto groundMat = std::make_shared<GLframework::PhongPointShadowMaterial>();
		groundMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/land.jpg", 0, GL_SRGB_ALPHA);

		auto groundGeo = GLframework::Geometry::createPlane(context.renderer->getShader(groundMat->getMaterialType()), 10.0, 10.0);
		auto groundMeshA = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		auto groundMeshB = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		auto groundMeshC = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		context.textD = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		auto groundMeshE = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		auto groundMeshF = std::make_shared<GLframework::Mesh>(groundGeo, groundMat);
		groundMeshA->setName("Floor");
		groundMeshB->setName("Ceiling");
		groundMeshC->setName("Front Wall");
		context.textD->setName("Back Wall");
		groundMeshE->setName("Left Wall");
		groundMeshF->setName("Right Wall");

		groundMeshA->setPosition({ 0.0f, -5.0f, 0.0f });
		groundMeshA->rotateX(-90);

		groundMeshB->setPosition({ 0.0f, 5.0f, 0.0f });
		groundMeshB->rotateX(90);

		groundMeshC->setPosition({ 0.0f, 0.0f, -5.0f });

		context.textD->setPosition({ 0.0f, 0.0f, 5.0f });
		context.textD->rotateX(180);

		groundMeshE->setPosition({ -5.0f, 0.0f, 0.0f });
		groundMeshE->rotateY(90);

		groundMeshF->setPosition({ 5.0f, 0.0f, 0.0f });
		groundMeshF->rotateY(-90);

		context.sceneOffScreen->addChild(groundMeshA);
		context.sceneOffScreen->addChild(groundMeshB);
		context.sceneOffScreen->addChild(groundMeshC);
		context.sceneOffScreen->addChild(context.textD);
		context.sceneOffScreen->addChild(groundMeshE);
		context.sceneOffScreen->addChild(groundMeshF);

		auto boxMat = std::make_shared<GLframework::PhongPointShadowMaterial>();
		boxMat->mDiffuse = std::make_shared<GLframework::Texture>("Texture/box.png", 0, GL_SRGB_ALPHA);
		auto boxGeo = GLframework::Geometry::createBox(context.renderer->getShader(boxMat->getMaterialType()), 3, 1, 1);
		auto boxMeshA = std::make_shared<GLframework::Mesh>(boxGeo, boxMat);
		boxMeshA->setName("Center Box");
		context.sceneOffScreen->addChild(boxMeshA);
	}

	void preparePBRPreview(GL_SCENE::SetupContext& context)
	{
		auto pbrMat = std::make_shared<GLframework::PBRMaterial>();
		pbrMat->mAlbedo = { 0.9f, 0.42f, 0.18f };
		pbrMat->mMetallic = 0.2f;
		pbrMat->mRoughness = 0.35f;
		pbrMat->mAo = 1.0f;
		pbrMat->mNormalMap = std::make_shared<GLframework::Texture>("Texture/normal/normal_map.png", 4);

		auto pbrGeo = GLframework::Geometry::createSphere(
			context.renderer->getShader(pbrMat->getMaterialType()),
			0.75f,
			48,
			24
		);
		auto pbrMesh = std::make_shared<GLframework::Mesh>(pbrGeo, pbrMat);
		pbrMesh->setName("PBR Preview Sphere");
		pbrMesh->setPosition({ 2.2f, -3.9f, 2.0f });
		context.sceneOffScreen->addChild(pbrMesh);
	}

	void prepareScreenPass(GL_SCENE::SetupContext& context)
	{
		context.screenMaterial = std::make_shared<GLframework::ScreenMaterial>();
		context.screenMaterial->mScreenTexture = context.framebufferResolve->getColorAttachment();
		auto geo = GLframework::Geometry::createScreenPlane(context.renderer->getShader(context.screenMaterial->getMaterialType()));
		auto mesh = std::make_shared<GLframework::Mesh>(geo, context.screenMaterial);
		mesh->setName("Screen Quad");
		context.sceneInScreen->addChild(mesh);
	}

	void prepareLights(GL_SCENE::SetupContext& context)
	{
		context.spotLight = std::make_shared<GLframework::SpotLight>(30.0f, 60.0f);
		context.spotLight->setName("Spot Light");
		context.spotLight->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));
		context.spotLight->setColor(glm::vec3{ 0.0f });

		context.dirLight = std::make_shared<GLframework::DirectionalLight>();
		context.dirLight->setName("Directional Light");
		context.dirLight->setPosition(glm::vec3(0.0f, 11.0f, 0.0f));
		context.dirLight->rotateX(-45.0f);
		context.dirLight->rotateY(45.0f);
		context.dirLight->setColor({ 0.0f, 0.0f, 0.0f });
		context.dirLight->setSpecularIntensity(0.5f);

		context.ambientLight = std::make_shared<GLframework::AmbientLight>();
		context.ambientLight->setColor(glm::vec3(0.1f));

		context.pointLights.clear();
		for (int i = 0; i < 2; ++i)
		{
			auto pointLight = std::make_shared<GLframework::PointLight>();
			pointLight->setSpecularIntensity(0.9f);
			pointLight->setK(0.0f, 0.0f, 1.0f);

			if (i == 0)
			{
				pointLight->setName("Point Light 0");
				pointLight->setPosition(glm::vec3(3.0f, 3.0f, -1.0f));
				pointLight->setColor(glm::vec3(0.8f, 0.8f, 0.9f));
			}
			else
			{
				pointLight->setName("Point Light 1");
				pointLight->setPosition(glm::vec3(-3.0f, 3.0f, -1.0f));
				pointLight->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
			}

			context.pointLights.push_back(std::move(pointLight));
		}

		GLframework::PointLightShadow::setMAX_POINT_LIGHT(static_cast<int>(context.pointLights.size()));
	}
}

void GL_SCENE::prepareDefaultScene(SetupContext& context)
{
	prepareRenderResources(context);
	prepareSkyBox(context);
	prepareRoomScene(context);
	preparePBRPreview(context);
	prepareScreenPass(context);
	prepareLights(context);
}
