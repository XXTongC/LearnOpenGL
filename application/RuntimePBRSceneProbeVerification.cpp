#include "RuntimePBRSceneProbeVerification.h"

#include <memory>
#include <string>

#include "RuntimeVerificationConfig.h"
#include "../framework/geometry.h"
#include "../framework/scene.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/renderer.h"
#include "AppRuntimeContext.h"

namespace
{
	std::shared_ptr<GLframework::Texture> loadLinearTexture(const std::string& path, unsigned int unit)
	{
		return std::make_shared<GLframework::Texture>(path, unit, GL_RGBA);
	}

	std::shared_ptr<GLframework::PBRMaterial> createPbrShowcaseMaterial(
		const glm::vec3& albedo,
		float metallic,
		float roughness
	)
	{
		auto material = std::make_shared<GLframework::PBRMaterial>();
		material->mAlbedo = albedo;
		material->mMetallic = metallic;
		material->mRoughness = roughness;
		material->mAo = 1.0f;
		material->mUseIBL = true;
		material->mIblDiffuseStrength = 1.15f;
		material->mIblSpecularStrength = 1.25f;
		return material;
	}

	void addPbrShowcaseSphere(
		GLframework::AppRuntimeContext& context,
		const std::shared_ptr<GLframework::Geometry>& geometry,
		const std::shared_ptr<GLframework::PBRMaterial>& material,
		const std::string& label,
		const glm::vec3& position,
		const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }
	)
	{
		auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
		mesh->setName("PBR Showcase Sphere " + label);
		mesh->setPosition(position);
		mesh->setScale(scale);
		context.renderResources.sceneOffScreen()->addChild(mesh);
	}

	void addPbrShowcaseSpheres(GLframework::AppRuntimeContext& context)
	{
		if (!context.renderResources.sceneOffScreen() || !context.renderResources.renderer())
		{
			return;
		}

		auto sphereGeometry = GLframework::Geometry::createSphere(
			context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
			0.42f,
			48,
			24
		);

		auto earth = createPbrShowcaseMaterial({ 1.0f, 1.0f, 1.0f }, 0.0f, 0.52f);
		earth->mAlbedoMap = GLframework::Texture::createTexture("Texture/solar system/2k_earth_daymap.jpg", 0);
		addPbrShowcaseSphere(context, sphereGeometry, earth, "Earth Albedo IBL", { -2.65f, 1.05f, 2.05f });

		auto mars = createPbrShowcaseMaterial({ 1.0f, 0.82f, 0.68f }, 0.0f, 0.68f);
		mars->mAlbedoMap = GLframework::Texture::createTexture("Texture/solar system/2k_mars.jpg", 0);
		addPbrShowcaseSphere(context, sphereGeometry, mars, "Mars Rough Dielectric", { -1.55f, 1.05f, 1.98f });

		auto brushedTextureSet = createPbrShowcaseMaterial({ 1.0f, 1.0f, 1.0f }, 0.0f, 0.48f);
		brushedTextureSet->mAlbedoMap = GLframework::Texture::createTexture("fbx/bag/diffuse.jpg", 0);
		brushedTextureSet->mMetallicMap = loadLinearTexture("fbx/bag/specular.jpg", 1);
		brushedTextureSet->mRoughnessMap = loadLinearTexture("fbx/bag/roughness.jpg", 2);
		brushedTextureSet->mAoMap = loadLinearTexture("fbx/bag/ao.jpg", 3);
		brushedTextureSet->mNormalMap = loadLinearTexture("fbx/bag/normal.png", 4);
		addPbrShowcaseSphere(context, sphereGeometry, brushedTextureSet, "Texture Set Normal Roughness AO", { -0.35f, 1.05f, 1.9f });

		auto gold = createPbrShowcaseMaterial({ 1.0f, 0.78f, 0.28f }, 1.0f, 0.18f);
		addPbrShowcaseSphere(context, sphereGeometry, gold, "Gold Metallic Low Roughness", { 0.85f, 1.05f, 1.9f });

		auto ceramic = createPbrShowcaseMaterial({ 0.12f, 0.72f, 1.0f }, 0.0f, 0.16f);
		ceramic->mNormalMap = loadLinearTexture("Texture/normal/normal_map.png", 4);
		addPbrShowcaseSphere(context, sphereGeometry, ceramic, "Glossy Normal Map", { 1.95f, 1.05f, 1.98f });

		auto emissiveSun = createPbrShowcaseMaterial({ 1.0f, 1.0f, 1.0f }, 0.0f, 0.9f);
		auto sunTexture = GLframework::Texture::createTexture("Texture/solar system/2k_sun.jpg", 0);
		emissiveSun->mAlbedoMap = sunTexture;
		emissiveSun->mEmissiveMap = sunTexture;
		emissiveSun->mEmissiveColor = { 1.0f, 0.52f, 0.16f };
		emissiveSun->mEmissiveIntensity = 2.4f;
		emissiveSun->mUseIBL = false;
		addPbrShowcaseSphere(context, sphereGeometry, emissiveSun, "Emissive Bloom", { 2.95f, 1.05f, 2.12f });
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRSceneProbeVerification::addVerificationSceneProbes(
		GLframework::AppRuntimeContext& context,
		const RuntimeVerificationConfig& verification
	)
	{
		const auto& probes = verification.pbr.probes;

		if ((!probes.enablePbrTransparentFallbackPass
			&& !probes.enablePbrEmissiveProbe
			&& !probes.enablePbrMaterialIblProbe
			&& !probes.enablePbrAlphaMaskProbe
			&& !probes.enablePbrTextureSetProbe
			&& !probes.enablePbrShowcaseSpheres) || !context.renderResources.sceneOffScreen() || !context.renderResources.renderer())
		{
			return;
		}

		if (probes.enablePbrTransparentFallbackPass)
		{
			auto material = std::make_shared<GLframework::PBRMaterial>();
			material->mAlbedo = { 0.15f, 0.85f, 1.0f };
			material->mMetallic = 0.0f;
			material->mRoughness = 0.18f;
			material->mAo = 1.0f;
			material->mUseIBL = true;
			material->mIblDiffuseStrength = 1.0f;
			material->mIblSpecularStrength = 1.0f;
			material->setColorBlendState(true);
			material->setOpacity(0.45f);
			material->setDepthWrite(false);

			auto geometry = GLframework::Geometry::createSphere(
				context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
				0.55f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Transparent Fallback Probe");
			mesh->setPosition({ 0.0f, 0.65f, 2.45f });
			context.renderResources.sceneOffScreen()->addChild(mesh);
		}

		if (probes.enablePbrEmissiveProbe)
		{
			auto material = std::make_shared<GLframework::PBRMaterial>();
			material->mAlbedo = { 0.0f, 0.0f, 0.0f };
			material->mMetallic = 0.0f;
			material->mRoughness = 1.0f;
			material->mAo = 1.0f;
			material->mEmissiveColor = { 0.0f, 0.85f, 1.0f };
			material->mEmissiveIntensity = 3.5f;
			material->mUseIBL = false;

			auto geometry = GLframework::Geometry::createSphere(
				context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
				0.48f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Emissive Probe");
			mesh->setPosition({ 0.0f, -0.7f, 2.35f });
			context.renderResources.sceneOffScreen()->addChild(mesh);
		}

		if (probes.enablePbrMaterialIblProbe)
		{
			auto material = std::make_shared<GLframework::PBRMaterial>();
			material->mAlbedo = { 0.95f, 0.78f, 0.22f };
			material->mMetallic = 0.0f;
			material->mRoughness = 0.35f;
			material->mAo = 1.0f;
			material->mUseIBL = true;
			material->mIblDiffuseStrength = 4.0f;
			material->mIblSpecularStrength = 4.0f;

			auto geometry = GLframework::Geometry::createSphere(
				context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
				0.52f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Material IBL Probe");
			mesh->setPosition({ 0.0f, 0.8f, 2.35f });
			context.renderResources.sceneOffScreen()->addChild(mesh);
		}

		if (probes.enablePbrAlphaMaskProbe)
		{
			auto material = std::make_shared<GLframework::PBRMaterial>();
			material->mAlbedo = { 1.0f, 1.0f, 1.0f };
			material->mAlbedoMap = GLframework::Texture::createTexture("Texture/window.png", 0);
			material->mMetallic = 0.0f;
			material->mRoughness = 0.45f;
			material->mAo = 1.0f;
			material->mUseAlphaMask = true;
			material->mAlphaCutoff = 0.5f;
			material->mUseIBL = true;
			material->mIblDiffuseStrength = 1.0f;
			material->mIblSpecularStrength = 1.0f;

			auto geometry = GLframework::Geometry::createPlane(
				context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
				1.25f,
				1.25f
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Alpha Mask Probe");
			mesh->setPosition({ 0.0f, 0.7f, 2.15f });
			context.renderResources.sceneOffScreen()->addChild(mesh);
		}

		if (probes.enablePbrTextureSetProbe)
		{
			auto material = std::make_shared<GLframework::PBRMaterial>();
			material->mAlbedo = { 1.0f, 1.0f, 1.0f };
			material->mAlbedoMap = GLframework::Texture::createTexture("fbx/bag/diffuse.jpg", 0);
			material->mMetallic = 0.0f;
			material->mMetallicMap = loadLinearTexture("fbx/bag/specular.jpg", 1);
			material->mRoughness = 0.5f;
			material->mRoughnessMap = loadLinearTexture("fbx/bag/roughness.jpg", 2);
			material->mAo = 1.0f;
			material->mAoMap = loadLinearTexture("fbx/bag/ao.jpg", 3);
			material->mNormalMap = loadLinearTexture("fbx/bag/normal.png", 4);
			material->mUseIBL = true;
			material->mIblDiffuseStrength = 1.0f;
			material->mIblSpecularStrength = 1.0f;

			auto geometry = GLframework::Geometry::createPlane(
				context.renderResources.renderer()->getShader(GLframework::MaterialType::PBRMaterial),
				1.55f,
				1.55f
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Texture Set Probe");
			mesh->setPosition({ 2.35f, -1.05f, 1.85f });
			context.renderResources.sceneOffScreen()->addChild(mesh);
		}

		if (probes.enablePbrShowcaseSpheres)
		{
			addPbrShowcaseSpheres(context);
		}
	}
}
