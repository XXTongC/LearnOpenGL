#include "SceneSetup.h"

#include <algorithm>
#include <string>

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

		context.frameRenderTargets.initialize(context.width, context.height, 4);
		context.bloom = std::make_shared<GLframework::Bloom>(context.width, context.height);

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

	float interpolatePreviewValue(float minValue, float maxValue, int index, int count)
	{
		if (count <= 1)
		{
			return minValue;
		}

		const float t = static_cast<float>(index) / static_cast<float>(count - 1);
		return minValue + (maxValue - minValue) * t;
	}

	std::shared_ptr<GLframework::PBRMaterial> createPBRPreviewMaterial(
		const GL_SCENE::PBRPreviewProfile& profile,
		const std::shared_ptr<GLframework::Texture>& normalMap,
		float metallic,
		float roughness
	)
	{
		auto pbrMat = std::make_shared<GLframework::PBRMaterial>();
		profile.material.applyTo(*pbrMat);
		pbrMat->mMetallic = metallic;
		pbrMat->mRoughness = roughness;
		pbrMat->mNormalMap = normalMap;
		return pbrMat;
	}

	void addPBRPreviewSphere(
		GL_SCENE::SetupContext& context,
		const std::shared_ptr<GLframework::Geometry>& geometry,
		const std::shared_ptr<GLframework::PBRMaterial>& material,
		const std::string& name,
		const glm::vec3& position
	)
	{
		auto pbrMesh = std::make_shared<GLframework::Mesh>(geometry, material);
		pbrMesh->setName(name);
		pbrMesh->setPosition(position);
		context.sceneOffScreen->addChild(pbrMesh);
	}

	void preparePBRPreview(GL_SCENE::SetupContext& context)
	{
		const auto& profile = context.pbrPreviewProfile;
		if (!profile.enabled)
		{
			return;
		}

		std::shared_ptr<GLframework::Texture> normalMap{ nullptr };
		if (!profile.normalMapPath.empty())
		{
			normalMap = std::make_shared<GLframework::Texture>(profile.normalMapPath, profile.normalMapUnit);
		}

		if (profile.useMaterialGrid)
		{
			const int columns = std::clamp(profile.gridColumns, 1, 10);
			const int rows = std::clamp(profile.gridRows, 1, 10);
			const float radius = std::max(profile.gridRadius, 0.01f);
			const float spacing = std::max(profile.gridSpacing, radius * 2.1f);
			auto pbrGeo = GLframework::Geometry::createSphere(
				context.renderer->getShader(GLframework::MaterialType::PBRMaterial),
				radius,
				std::max(profile.segments, 3),
				std::max(profile.rings, 2)
			);

			for (int row = 0; row < rows; ++row)
			{
				const float roughness = interpolatePreviewValue(profile.gridRoughnessMin, profile.gridRoughnessMax, row, rows);
				for (int column = 0; column < columns; ++column)
				{
					const float metallic = interpolatePreviewValue(profile.gridMetallicMin, profile.gridMetallicMax, column, columns);
					auto pbrMat = createPBRPreviewMaterial(profile, normalMap, metallic, roughness);
					const glm::vec3 offset{
						(static_cast<float>(column) - static_cast<float>(columns - 1) * 0.5f) * spacing,
						0.0f,
						(static_cast<float>(row) - static_cast<float>(rows - 1) * 0.5f) * spacing
					};
					addPBRPreviewSphere(
						context,
						pbrGeo,
						pbrMat,
						"PBR Preview M" + std::to_string(column) + " R" + std::to_string(row),
						profile.position + offset
					);
				}
			}
			return;
		}

		auto pbrMat = createPBRPreviewMaterial(profile, normalMap, profile.material.metallic, profile.material.roughness);
		auto pbrGeo = GLframework::Geometry::createSphere(
			context.renderer->getShader(pbrMat->getMaterialType()),
			std::max(profile.radius, 0.01f),
			std::max(profile.segments, 3),
			std::max(profile.rings, 2)
		);
		addPBRPreviewSphere(context, pbrGeo, pbrMat, "PBR Preview Sphere", profile.position);
	}

	void prepareScreenPass(GL_SCENE::SetupContext& context)
	{
		context.screenMaterial = std::make_shared<GLframework::ScreenMaterial>();
		context.screenMaterial->mScreenTexture = context.frameRenderTargets.getResolvedColorAttachment();
		context.screenMaterial->mDepthStencilTexture = context.frameRenderTargets.getResolvedDepthStencilAttachment();
		context.screenMaterial->mBloomTexture = context.frameRenderTargets.getBloomPongColorAttachment();
		auto geo = GLframework::Geometry::createScreenPlane(context.renderer->getShader(context.screenMaterial->getMaterialType()));
		context.screenQuad = std::make_shared<GLframework::Mesh>(geo, context.screenMaterial);
		context.screenQuad->setName("Screen Quad");
		context.sceneInScreen->addChild(context.screenQuad);
	}

	void prepareEnvironmentPrecompute(GL_SCENE::SetupContext& context)
	{
		if (!context.environmentProfile.precomputeOnPrepare)
		{
			return;
		}

		if (!context.environmentProfile.hasEnvironmentSource())
		{
			LogInfo("IBL precompute skipped: no HDR or procedural environment source configured");
			return;
		}

		if (context.renderer->precomputeEnvironment(context.environmentProfile))
		{
			LogInfo("IBL precompute finished");
			return;
		}

		LogInfo("IBL precompute failed");
	}

	void prepareLights(GL_SCENE::SetupContext& context)
	{
		context.lightRigProfile.applyTo(
			context.ambientLight,
			context.dirLight,
			context.spotLight,
			context.pointLights
		);
	}
}

void GL_SCENE::prepareDefaultScene(SetupContext& context)
{
	prepareRenderResources(context);
	prepareSkyBox(context);
	prepareRoomScene(context);
	preparePBRPreview(context);
	prepareScreenPass(context);
	prepareEnvironmentPrecompute(context);
	prepareLights(context);
}
