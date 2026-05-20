#include "RuntimePBRVerification.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../framework/object.h"
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
	};

	void reportLine(const std::string& message)
	{
		std::cout << message << std::endl;
		LogInfo(message);
	}

	void collectSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		PBRVerificationSceneStats& stats
	)
	{
		if (!object)
		{
			return;
		}

		++stats.objectCount;
		if (object->getType() == GLframework::ObjectType::Mesh)
		{
			++stats.meshCount;
		}

		const auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object);
		if (mesh && mesh->getMaterial() && mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
		{
			++stats.pbrMeshCount;
		}

		if (object->getName().find("PBR Preview") == 0)
		{
			++stats.pbrPreviewMeshCount;
		}

		for (const auto& child : object->getChildren())
		{
			collectSceneStats(child, stats);
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
	void RuntimePBRVerification::applyProfile(GLframework::AppRuntimeContext& context)
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

		context.pbrPreviewProfile.enabled = true;
		context.pbrPreviewProfile.position = { 0.0f, -3.7f, 1.2f };
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

		reportLine("PBR verification profile applied: procedural IBL + 5x5 material grid");
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
		reportLine(
			"PBR verification renderer stats: rendererPasses=" + std::to_string(stats.rendererPassCount)
			+ ", shadowCasters=" + std::to_string(stats.shadowCasterCount)
			+ ", directionalShadowLayers=" + std::to_string(stats.directionalShadowLayerCount)
			+ ", directionalShadowDrawCalls=" + std::to_string(stats.directionalShadowDrawCalls)
			+ ", pointShadowLights=" + std::to_string(stats.pointShadowLightCount)
			+ ", pointShadowFaces=" + std::to_string(stats.pointShadowFaceCount)
			+ ", pointShadowDrawCalls=" + std::to_string(stats.pointShadowDrawCalls)
			+ ", pbrDepthPrepassDrawCalls=" + std::to_string(stats.pbrDepthPrepassDrawCalls)
			+ ", legacyDrawCalls=" + std::to_string(stats.legacySceneDrawCalls)
			+ ", pbrDrawCalls=" + std::to_string(stats.pbrSceneDrawCalls)
		);
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
