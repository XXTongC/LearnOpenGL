#include "RuntimePBRVerification.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "assimpLoader.h"
#include "../framework/object.h"
#include "../framework/geometry.h"
#include "../framework/scene.h"
#include "../materials/material.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/renderer.h"
#include "../tools/Logger/LogManager.h"
#include "AppRuntimeContext.h"
#include "GL_ERROR_FIND.h"
#include "core.h"

namespace
{
	struct PBRVerificationSceneStats
	{
		int objectCount{ 0 };
		int meshCount{ 0 };
		int pbrMeshCount{ 0 };
		int pbrPreviewMeshCount{ 0 };
		int transparentMeshCount{ 0 };
		int pbrTransparentMeshCount{ 0 };
		int pbrEmissiveMeshCount{ 0 };
		int pbrCustomIblMeshCount{ 0 };
		int pbrAlphaMaskedMeshCount{ 0 };
		int pbrImportedMeshCount{ 0 };
	};

	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	void collectSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		PBRVerificationSceneStats& stats,
		bool importedAssetSubtree = false
	)
	{
		if (!object)
		{
			return;
		}

		++stats.objectCount;
		const bool importedAsset = importedAssetSubtree || object->getName().find("PBR Imported") == 0;
		if (object->getType() == GLframework::ObjectType::Mesh)
		{
			++stats.meshCount;
		}

		const auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object);
		if (mesh && mesh->getMaterial() && mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
		{
			++stats.pbrMeshCount;
			if (importedAsset)
			{
				++stats.pbrImportedMeshCount;
			}
			const auto pbrMaterial = std::dynamic_pointer_cast<GLframework::PBRMaterial>(mesh->getMaterial());
			if (pbrMaterial && pbrMaterial->mEmissiveIntensity > 0.0f && glm::length(pbrMaterial->mEmissiveColor) > 0.0001f)
			{
				++stats.pbrEmissiveMeshCount;
			}
			if (pbrMaterial && pbrMaterial->mUseAlphaMask)
			{
				++stats.pbrAlphaMaskedMeshCount;
			}
			if (pbrMaterial
				&& pbrMaterial->mUseIBL
				&& (pbrMaterial->mIblDiffuseStrength < 0.999f
					|| pbrMaterial->mIblDiffuseStrength > 1.001f
					|| pbrMaterial->mIblSpecularStrength < 0.999f
					|| pbrMaterial->mIblSpecularStrength > 1.001f))
			{
				++stats.pbrCustomIblMeshCount;
			}
		}
		if (mesh && mesh->getMaterial() && mesh->getMaterial()->getColorBlendState())
		{
			++stats.transparentMeshCount;
			if (mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
			{
				++stats.pbrTransparentMeshCount;
			}
		}

		if (object->getName().find("PBR Preview") == 0)
		{
			++stats.pbrPreviewMeshCount;
		}

		for (const auto& child : object->getChildren())
		{
			collectSceneStats(child, stats, importedAsset);
		}
	}

	void writePpmRows(
		std::ofstream& output,
		const std::vector<unsigned char>& pixels,
		unsigned int width,
		unsigned int height
	)
	{
		const auto rowStride = static_cast<std::streamsize>(width * 3);
		for (int row = static_cast<int>(height) - 1; row >= 0; --row)
		{
			const auto rowOffset = static_cast<std::streamoff>(row) * rowStride;
			output.write(
				reinterpret_cast<const char*>(pixels.data() + rowOffset),
				rowStride
			);
		}
	}
}

namespace GL_RUNTIME
{
	void RuntimePBRVerification::applyProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimePBRVerificationConfig& config
	)
	{
		context.environmentProfile.precomputeOnPrepare = true;
		context.environmentProfile.useProceduralEnvironment = true;
		context.environmentProfile.proceduralWidth = 128;
		context.environmentProfile.proceduralHeight = 64;
		context.environmentProfile.proceduralSkyIntensity = 1.5f;
		context.environmentProfile.proceduralGroundIntensity = 0.15f;
		context.environmentProfile.proceduralSunIntensity = 4.0f;

		context.postProcessSettings.exposure = 1.0f;
		context.postProcessSettings.bloomEnabled = true;
		context.postProcessSettings.bloomThreshold = 1.0f;
		context.postProcessSettings.bloomIntensity = 0.04f;
		context.postProcessSettings.bloomIterations = 6;

		context.framePipelineProfile.sceneColorPassEnabled = true;
		context.framePipelineProfile.sceneResolvePassEnabled = true;
		context.framePipelineProfile.bloomPassEnabled = true;
		context.framePipelineProfile.screenCompositePassEnabled = true;
		context.framePipelineProfile.passOrder = "SceneColor,SceneResolve,Bloom,ScreenComposite";
		applyRendererPassProfile(context, config);

		context.pbrPreviewProfile.enabled = true;
		context.pbrPreviewProfile.position = { 0.0f, 0.0f, 1.2f };
		context.pbrPreviewProfile.segments = 32;
		context.pbrPreviewProfile.rings = 16;
		context.pbrPreviewProfile.useMaterialGrid = true;
		context.pbrPreviewProfile.gridColumns = 5;
		context.pbrPreviewProfile.gridRows = 5;
		context.pbrPreviewProfile.gridSpacing = 0.9f;
		context.pbrPreviewProfile.gridRadius = 0.32f;
		context.pbrPreviewProfile.gridMetallicMin = 0.0f;
		context.pbrPreviewProfile.gridMetallicMax = 1.0f;
		context.pbrPreviewProfile.gridRoughnessMin = 0.08f;
		context.pbrPreviewProfile.gridRoughnessMax = 1.0f;
		context.pbrPreviewProfile.material.albedo = { 0.9f, 0.42f, 0.18f };
		context.pbrPreviewProfile.material.ao = 1.0f;
		context.pbrPreviewProfile.material.useIBL = true;
		context.pbrPreviewProfile.material.iblDiffuseStrength = 1.0f;
		context.pbrPreviewProfile.material.iblSpecularStrength = 1.0f;
		context.pbrPreviewProfile.normalMapPath = "Texture/normal/normal_map.png";
		context.pbrPreviewProfile.normalMapUnit = 4;

		context.pbrLightRigProfile.ambientColor = { 0.1f, 0.1f, 0.1f };
		context.pbrLightRigProfile.ambientIntensity = 1.0f;
		context.pbrLightRigProfile.directional.color = { 0.0f, 0.0f, 0.0f };
		context.pbrLightRigProfile.spot.color = { 0.0f, 0.0f, 0.0f };
		context.pbrLightRigProfile.pointLightCount = 2;
		context.pbrLightRigProfile.pointLights[0].position = { 3.0f, 3.0f, -1.0f };
		context.pbrLightRigProfile.pointLights[0].color = { 0.8f, 0.8f, 0.9f };
		context.pbrLightRigProfile.pointLights[0].intensity = 1.0f;
		context.pbrLightRigProfile.pointLights[0].specularIntensity = 0.9f;
		context.pbrLightRigProfile.pointLights[1].position = { -3.0f, 3.0f, -1.0f };
		context.pbrLightRigProfile.pointLights[1].color = { 1.0f, 1.0f, 1.0f };
		context.pbrLightRigProfile.pointLights[1].intensity = 1.0f;
		context.pbrLightRigProfile.pointLights[1].specularIntensity = 0.9f;

		context.pbrCameraRigProfile.position = { 0.0f, 0.0f, 5.0f };
		context.pbrCameraRigProfile.up = { 0.0f, 1.0f, 0.0f };
		context.pbrCameraRigProfile.right = { 1.0f, 0.0f, 0.0f };
		context.pbrCameraRigProfile.fovy = 60.0f;
		context.pbrCameraRigProfile.nearPlane = 0.1f;
		context.pbrCameraRigProfile.farPlane = 1000.0f;
		context.pbrCameraRigProfile.applyTo(context.camera);

		std::string profileLine = "PBR verification profile applied: procedural IBL + 5x5 material grid";
		if (config.enablePbrGBufferPass)
		{
			profileLine += " + PBR G-buffer pass";
		}
		if (config.enablePbrDeferredLightingPass)
		{
			profileLine += " + PBR deferred lighting pass";
		}
		if (config.enablePbrTransparentFallbackPass)
		{
			profileLine += " + transparent forward fallback";
		}
		if (config.enablePbrEmissiveProbe)
		{
			profileLine += " + emissive G-buffer probe";
		}
		if (config.enablePbrMaterialIblProbe)
		{
			profileLine += " + material IBL params probe";
		}
		if (config.enablePbrAlphaMaskProbe)
		{
			profileLine += " + alpha mask probe";
		}
		if (config.enablePbrImportedAssetProbe)
		{
			profileLine += " + imported PBR asset probe";
		}
		if (config.enablePbrGBufferDebugPass)
		{
			profileLine += " + PBR G-buffer debug pass";
		}
		if (config.enableIblDebugPass)
		{
			profileLine += " + IBL debug pass";
		}
		if (config.disablePbrShadowAtlasPass)
		{
			profileLine += " + PBR shadow atlas disabled";
		}
		reportLine(profileLine);
	}

	void RuntimePBRVerification::applyRendererPassProfile(
		GLframework::AppRuntimeContext& context,
		const RuntimePBRVerificationConfig& config
	)
	{
		if (!context.renderer)
		{
			return;
		}

		auto& rendererPassProfile = context.renderer->getFramePassProfile();
		rendererPassProfile.resetToDefaults();
		const std::string shadowPrefix = config.disablePbrShadowAtlasPass
			? "BeginFrame,ShadowMaps,"
			: "BeginFrame,ShadowMaps,PBRShadowAtlas,";
		if (config.disablePbrShadowAtlasPass)
		{
			rendererPassProfile.defaultPassOrder =
				"BeginFrame,ShadowMaps,PBRDepthPrepass,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene";
		}
		if (config.enablePbrDeferredLightingPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,PBRDeferredLighting";
			if (config.enablePbrTransparentFallbackPass)
			{
				rendererPassProfile.defaultPassOrder += ",LegacyTransparentScene,PBRTransparentScene";
			}
			rendererPassProfile.pbrDeferredLightingIntensity = 1.0f;
			rendererPassProfile.pbrDeferredIblDiffuseStrength = 1.0f;
			rendererPassProfile.pbrDeferredIblSpecularStrength = 1.0f;
		}
		else if (config.enablePbrGBufferPass || config.enablePbrGBufferDebugPass)
		{
			rendererPassProfile.defaultPassOrder =
				shadowPrefix + "PBRDepthPrepass,PBRGBuffer,LegacyOpaqueScene,PBROpaqueScene,LegacyTransparentScene,PBRTransparentScene";
		}

		if (config.enablePbrGBufferDebugPass)
		{
			rendererPassProfile.defaultPassOrder += ",PBRGBufferDebug";
			rendererPassProfile.pbrGBufferDebugMode = 0;
			rendererPassProfile.pbrGBufferDebugIntensity = 1.0f;
		}

		if (config.enableIblDebugPass)
		{
			rendererPassProfile.defaultPassOrder += ",IBLDebug";
			rendererPassProfile.iblDebugMode = 0;
			rendererPassProfile.iblDebugMipLevel = 0.0f;
			rendererPassProfile.iblDebugIntensity = 1.0f;
		}
	}

	void RuntimePBRVerification::addVerificationSceneProbes(
		GLframework::AppRuntimeContext& context,
		const RuntimePBRVerificationConfig& config
	)
	{
		if ((!config.enablePbrTransparentFallbackPass
			&& !config.enablePbrEmissiveProbe
			&& !config.enablePbrMaterialIblProbe
			&& !config.enablePbrAlphaMaskProbe
			&& !config.enablePbrImportedAssetProbe) || !context.sceneOffScreen || !context.renderer)
		{
			return;
		}

		if (config.enablePbrTransparentFallbackPass)
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
				context.renderer->getShader(GLframework::MaterialType::PBRMaterial),
				0.55f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Transparent Fallback Probe");
			mesh->setPosition({ 0.0f, 0.65f, 2.45f });
			context.sceneOffScreen->addChild(mesh);
		}

		if (config.enablePbrEmissiveProbe)
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
				context.renderer->getShader(GLframework::MaterialType::PBRMaterial),
				0.48f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Emissive Probe");
			mesh->setPosition({ 0.0f, -0.7f, 2.35f });
			context.sceneOffScreen->addChild(mesh);
		}

		if (config.enablePbrMaterialIblProbe)
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
				context.renderer->getShader(GLframework::MaterialType::PBRMaterial),
				0.52f,
				32,
				16
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Material IBL Probe");
			mesh->setPosition({ 0.0f, 0.8f, 2.35f });
			context.sceneOffScreen->addChild(mesh);
		}

		if (config.enablePbrAlphaMaskProbe)
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
				context.renderer->getShader(GLframework::MaterialType::PBRMaterial),
				1.25f,
				1.25f
			);
			auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
			mesh->setName("PBR Deferred Alpha Mask Probe");
			mesh->setPosition({ 0.0f, 0.7f, 2.15f });
			context.sceneOffScreen->addChild(mesh);
		}

		if (config.enablePbrImportedAssetProbe)
		{
			auto importedAsset = GL_APPLICATION::AssimpLoader::loadPBR("fbx/test/test.fbx", context.renderer);
			if (importedAsset)
			{
				importedAsset->setName("PBR Imported Asset Probe");
				importedAsset->setPosition({ -2.4f, -1.1f, 1.8f });
				importedAsset->setScale({ 0.65f, 0.65f, 0.65f });
				context.sceneOffScreen->addChild(importedAsset);
			}
			else
			{
				reportLine("PBR imported asset probe failed: fbx/test/test.fbx");
			}
		}
	}

	void RuntimePBRVerification::reportPreparedScene(GLframework::AppRuntimeContext& context)
	{
		PBRVerificationSceneStats stats{};
		collectSceneStats(std::static_pointer_cast<GLframework::Object>(context.sceneOffScreen), stats);
		const bool environmentReady = context.renderer
			&& context.renderer->getEnvironmentRenderTargets().hasPrecomputedEnvironment();

		reportLine(
			"PBR verification scene stats: objects=" + std::to_string(stats.objectCount)
			+ ", meshes=" + std::to_string(stats.meshCount)
			+ ", pbrMeshes=" + std::to_string(stats.pbrMeshCount)
			+ ", pbrPreviewMeshes=" + std::to_string(stats.pbrPreviewMeshCount)
			+ ", transparentMeshes=" + std::to_string(stats.transparentMeshCount)
			+ ", pbrTransparentMeshes=" + std::to_string(stats.pbrTransparentMeshCount)
			+ ", pbrEmissiveMeshes=" + std::to_string(stats.pbrEmissiveMeshCount)
			+ ", pbrCustomIblMeshes=" + std::to_string(stats.pbrCustomIblMeshCount)
			+ ", pbrAlphaMaskedMeshes=" + std::to_string(stats.pbrAlphaMaskedMeshCount)
			+ ", pbrImportedMeshes=" + std::to_string(stats.pbrImportedMeshCount)
			+ ", iblReady=" + (environmentReady ? std::string{ "yes" } : std::string{ "no" })
		);
	}

	void RuntimePBRVerification::reportRenderedFrame(GLframework::AppRuntimeContext& context)
	{
		if (!context.renderer)
		{
			return;
		}

		const auto& stats = context.renderer->getLastFrameStats();
		std::string statsLine =
			"PBR verification renderer stats: rendererPasses=" + std::to_string(stats.rendererPassCount)
			+ ", shadowCasters=" + std::to_string(stats.shadowCasterCount)
			+ ", directionalShadowLayers=" + std::to_string(stats.directionalShadowLayerCount)
			+ ", directionalShadowDrawCalls=" + std::to_string(stats.directionalShadowDrawCalls)
			+ ", directionalAlphaMaskedShadowDrawCalls=" + std::to_string(stats.directionalAlphaMaskedShadowDrawCalls)
			+ ", pointShadowLights=" + std::to_string(stats.pointShadowLightCount)
			+ ", pointShadowFaces=" + std::to_string(stats.pointShadowFaceCount)
			+ ", pointShadowDrawCalls=" + std::to_string(stats.pointShadowDrawCalls)
			+ ", pointAlphaMaskedShadowDrawCalls=" + std::to_string(stats.pointAlphaMaskedShadowDrawCalls)
			+ ", pbrShadowAtlasReady=" + (stats.pbrShadowAtlasReady ? std::string{ "yes" } : std::string{ "no" })
			+ ", pbrShadowAtlasDirectionalLayers=" + std::to_string(stats.pbrShadowAtlasDirectionalLayers)
			+ ", pbrShadowAtlasPointFaces=" + std::to_string(stats.pbrShadowAtlasPointFaces)
			+ ", pbrShadowAtlasPointFacesRendered=" + std::to_string(stats.pbrShadowAtlasPointFacesRendered)
			+ ", pbrShadowAtlasDirectionalDrawCalls=" + std::to_string(stats.pbrShadowAtlasDirectionalDrawCalls)
			+ ", pbrShadowAtlasPointDrawCalls=" + std::to_string(stats.pbrShadowAtlasPointDrawCalls)
			+ ", pbrShadowAtlasDirectionalAlphaMaskedDrawCalls=" + std::to_string(stats.pbrShadowAtlasDirectionalAlphaMaskedDrawCalls)
			+ ", pbrShadowAtlasPointAlphaMaskedDrawCalls=" + std::to_string(stats.pbrShadowAtlasPointAlphaMaskedDrawCalls)
			+ ", pbrDepthPrepassDrawCalls=" + std::to_string(stats.pbrDepthPrepassDrawCalls)
			+ ", legacyDrawCalls=" + std::to_string(stats.legacySceneDrawCalls)
			+ ", legacyTransparentDrawCalls=" + std::to_string(stats.legacyTransparentDrawCalls)
			+ ", pbrDrawCalls=" + std::to_string(stats.pbrSceneDrawCalls)
			+ ", pbrTransparentDrawCalls=" + std::to_string(stats.pbrTransparentDrawCalls);
		if (stats.iblDebugDrawCalls > 0)
		{
			statsLine += ", iblDebugDrawCalls=" + std::to_string(stats.iblDebugDrawCalls);
		}
		if (stats.pbrGBufferReady || stats.pbrGBufferDrawCalls > 0)
		{
			statsLine += ", pbrGBufferDrawCalls=" + std::to_string(stats.pbrGBufferDrawCalls);
			statsLine += ", pbrGBufferReady=";
			statsLine += (stats.pbrGBufferReady ? "yes" : "no");
			statsLine += ", pbrGBufferSize=" + std::to_string(stats.pbrGBufferWidth)
				+ "x" + std::to_string(stats.pbrGBufferHeight);
		}
		if (stats.pbrDeferredLightingDrawCalls > 0)
		{
			statsLine += ", pbrDeferredLightingDrawCalls=" + std::to_string(stats.pbrDeferredLightingDrawCalls);
			statsLine += ", pbrDeferredCsmShadowBound=";
			statsLine += (stats.pbrDeferredCsmShadowBound ? "yes" : "no");
			statsLine += ", pbrDeferredCsmShadowLayers=" + std::to_string(stats.pbrDeferredCsmShadowLayers);
			statsLine += ", pbrDeferredCsmShadowAtlasBound=";
			statsLine += (stats.pbrDeferredCsmShadowAtlasBound ? "yes" : "no");
			statsLine += ", pbrDeferredPointShadowAtlasBound=";
			statsLine += (stats.pbrDeferredPointShadowAtlasBound ? "yes" : "no");
			statsLine += ", pbrDeferredPointShadowAtlasLights=" + std::to_string(stats.pbrDeferredPointShadowAtlasLights);
			statsLine += ", pbrDeferredLightBufferBound=";
			statsLine += (stats.pbrDeferredLightBufferBound ? "yes" : "no");
			statsLine += ", pbrDeferredLightBufferPointLights=" + std::to_string(stats.pbrDeferredLightBufferPointLights);
			statsLine += "/" + std::to_string(stats.pbrDeferredLightBufferMaxPointLights);
		}
		if (stats.pbrGBufferDebugDrawCalls > 0)
		{
			statsLine += ", pbrGBufferDebugDrawCalls=" + std::to_string(stats.pbrGBufferDebugDrawCalls);
		}
		reportLine(statsLine);
	}

	bool RuntimePBRVerification::captureDefaultFramebuffer(
		const std::string& path,
		unsigned int width,
		unsigned int height
	)
	{
		if (width == 0 || height == 0 || path.empty())
		{
			return false;
		}

		std::vector<unsigned char> pixels(static_cast<std::size_t>(width) * height * 3);
		GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
		GL_CALL(glReadBuffer(GL_BACK));
		GL_CALL(glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data()));

		const std::filesystem::path capturePath{ path };
		const auto parentPath = capturePath.parent_path();
		if (!parentPath.empty())
		{
			std::error_code error{};
			std::filesystem::create_directories(parentPath, error);
			if (error)
			{
				reportLine("PBR verification capture failed: cannot create " + parentPath.string());
				return false;
			}
		}

		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output)
		{
			reportLine("PBR verification capture failed: cannot write " + path);
			return false;
		}

		output << "P6\n" << width << ' ' << height << "\n255\n";
		writePpmRows(output, pixels, width, height);
		reportLine("PBR verification capture written: " + path);
		return true;
	}
}
