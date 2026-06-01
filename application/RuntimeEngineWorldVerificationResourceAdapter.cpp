#include "RuntimeEngineWorldVerificationResourceAdapter.h"

#include <memory>
#include <utility>

#include "../engine/AssetRegistry.h"
#include "../engine/ScenePackage.h"
#include "../framework/geometry.h"
#include "../framework/object.h"
#include "../framework/scene.h"
#include "../light/pointLight.h"
#include "../materials/material.h"
#include "../materials/pbrMaterial/PBRMaterial.h"
#include "../mesh/mesh.h"
#include "../renderer/renderer.h"
#include "RuntimeRenderResourceState.h"

namespace
{
	void collectEngineWorldPreparedSceneStats(
		const std::shared_ptr<GLframework::Object>& object,
		GL_RUNTIME::RuntimeEngineWorldPreparedSceneStatsSnapshot& stats
	)
	{
		if (!object)
		{
			return;
		}

		const bool engineWorldProbe = object->getName().find("Engine World Scene Probe") == 0;
		const bool engineWorldMinimal = object->getName().find("Engine World Minimal") == 0;
		const auto mesh = std::dynamic_pointer_cast<GLframework::Mesh>(object);
		if (mesh
			&& mesh->getMaterial()
			&& mesh->getMaterial()->getMaterialType() == GLframework::MaterialType::PBRMaterial)
		{
			if (engineWorldProbe)
			{
				++stats.engineWorldProbeMeshCount;
			}
			if (engineWorldMinimal)
			{
				++stats.engineWorldMinimalMeshCount;
			}
		}

		for (const auto& child : object->getChildren())
		{
			collectEngineWorldPreparedSceneStats(child, stats);
		}
	}

	std::shared_ptr<GLframework::PBRMaterial> createResolvedScenePackagePbrMaterial(
		const GLengine::ScenePackageAdapterDescriptor& descriptor
	)
	{
		auto material = std::make_shared<GLframework::PBRMaterial>();
		material->setSurface({
			{ 0.85f, 0.85f, 0.85f },
			{ 0.0f, 0.0f, 0.0f },
			0.0f,
			0.5f,
			1.0f,
			0.0f
		});
		material->setIbl({ true, 1.0f, 1.0f });

		if (descriptor.objectName.find("Matte") != std::string::npos)
		{
			material->setAlbedo({ 0.92f, 0.38f, 0.18f });
			material->setMetallic(0.0f);
			material->setRoughness(0.72f);
		}
		else if (descriptor.objectName.find("Metallic") != std::string::npos)
		{
			material->setAlbedo({ 1.0f, 0.78f, 0.28f });
			material->setMetallic(1.0f);
			material->setRoughness(0.22f);
			material->setIblStrengths(1.0f, 1.25f);
		}
		else if (descriptor.objectName.find("Gloss") != std::string::npos)
		{
			material->setAlbedo({ 0.16f, 0.62f, 1.0f });
			material->setMetallic(0.0f);
			material->setRoughness(0.16f);
			material->setIblStrengths(1.0f, 1.3f);
		}
		else if (descriptor.objectName.find("Emissive") != std::string::npos)
		{
			material->setSurface({
				{ 0.02f, 0.02f, 0.02f },
				{ 0.25f, 0.95f, 0.8f },
				0.0f,
				0.95f,
				1.0f,
				2.4f
			});
			material->setIbl({ false, 0.0f, 0.0f });
		}

		return material;
	}

	class RuntimeScenePackageAssetResolver : public GLengine::ScenePackageAssetResolver
	{
	public:
		explicit RuntimeScenePackageAssetResolver(std::shared_ptr<GLframework::Renderer> renderer)
			: mRenderer(std::move(renderer))
		{
		}

		std::shared_ptr<GLframework::Mesh> resolveMesh(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (!mRenderer
				|| descriptor.adapterKind != "mesh"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::Mesh,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::Mesh,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:mesh:") != 0))
			{
				return nullptr;
			}

			auto geometry = GLframework::Geometry::createSphere(
				mRenderer->getShader(GLframework::MaterialType::PBRMaterial),
				0.36f,
				40,
				20
			);
			auto mesh = std::make_shared<GLframework::Mesh>(
				geometry,
				createResolvedScenePackagePbrMaterial(descriptor)
			);
			mesh->setName(descriptor.objectName.empty() ? descriptor.assetReference : descriptor.objectName);
			return mesh;
		}

		std::shared_ptr<GLframework::Object> resolveLegacyObject(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (descriptor.adapterKind != "legacy-object"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::LegacyObject,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::LegacyObject,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:legacy-object:") != 0))
			{
				return nullptr;
			}

			auto object = std::make_shared<GLframework::Object>();
			object->setName(descriptor.objectName.empty() ? "Resolved Legacy Object" : descriptor.objectName);
			return object;
		}

		std::shared_ptr<GLframework::Light> resolveLight(
			const GLengine::ScenePackageAdapterDescriptor& descriptor
		) override
		{
			if (descriptor.adapterKind != "light"
				|| (!GLengine::assetHandleMatches(
						descriptor.assetHandle,
						GLengine::AssetKind::Light,
						"runtime-generated"
					)
					&& !GLengine::assetHandleMatches(
						descriptor.assetReference,
						GLengine::AssetKind::Light,
						"runtime-generated"
					)
					&& descriptor.assetReference.find("runtime-generated:light:") != 0))
			{
				return nullptr;
			}

			auto pointLight = std::make_shared<GLframework::PointLight>();
			pointLight->setName(descriptor.objectName.empty() ? "Resolved Package Point Light" : descriptor.objectName);
			pointLight->setColor({ 1.0f, 0.78f, 0.45f });
			pointLight->setIntensity(1.8f);
			pointLight->setSpecularIntensity(1.0f);
			pointLight->setK(0.09f, 0.07f, 1.0f);
			return pointLight;
		}

	private:
		std::shared_ptr<GLframework::Renderer> mRenderer{ nullptr };
	};
}

GL_RUNTIME::RuntimeEngineWorldPreparedSceneStatsSnapshot
GL_RUNTIME::RuntimeEngineWorldVerificationResourceAdapter::collectPreparedSceneStats(
	const RuntimeRenderResourceState& renderResources
)
{
	RuntimeEngineWorldPreparedSceneStatsSnapshot stats{};
	collectEngineWorldPreparedSceneStats(
		std::static_pointer_cast<GLframework::Object>(renderResources.sceneOffScreen()),
		stats
	);
	return stats;
}

GLengine::ScenePackageLoadResult GL_RUNTIME::RuntimeEngineWorldVerificationResourceAdapter::loadRuntimeGeneratedScenePackage(
	const RuntimeRenderResourceState& renderResources,
	const std::string& path
)
{
	RuntimeScenePackageAssetResolver resolver(renderResources.renderer());
	GLengine::ScenePackageLoadOptions loadOptions{};
	loadOptions.assetResolver = &resolver;
	return GLengine::loadScenePackage(path, loadOptions);
}
