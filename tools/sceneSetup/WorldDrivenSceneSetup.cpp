#include "WorldDrivenSceneSetup.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "../../engine/ActorAdapters.h"
#include "../../engine/Level.h"
#include "../../engine/PersistentIdPolicy.h"
#include "../../engine/SceneComponent.h"
#include "../../engine/Transform.h"
#include "../../engine/World.h"
#include "../../engine/WorldLegacySceneExporter.h"
#include "../../framework/geometry.h"
#include "../../framework/object.h"
#include "../../framework/scene.h"
#include "../../materials/pbrMaterial/PBRMaterial.h"
#include "../../mesh/mesh.h"
#include "../../renderer/renderer.h"

namespace
{
	constexpr std::string_view kSceneProbePreset = "engine-world-scene-probe";
	constexpr std::string_view kMinimalScenePreset = "engine-world-minimal-scene";

	std::string makePresetPersistentId(
		std::string_view objectKind,
		std::string_view preset,
		std::initializer_list<std::string_view> pathSegments = {}
	)
	{
		return GLengine::makePersistentId(
			objectKind,
			GLengine::PersistentIdSource::PresetAssigned,
			preset,
			pathSegments
		);
	}

	std::shared_ptr<GLframework::PBRMaterial> createPbrMaterial(
		const glm::vec3& albedo,
		float metallic,
		float roughness,
		float iblDiffuseStrength = 1.0f,
		float iblSpecularStrength = 1.0f
	)
	{
		auto material = std::make_shared<GLframework::PBRMaterial>();
		material->setSurface({
			albedo,
			{ 0.0f, 0.0f, 0.0f },
			std::clamp(metallic, 0.0f, 1.0f),
			std::clamp(roughness, 0.04f, 1.0f),
			1.0f,
			0.0f
		});
		material->setIbl({ true, iblDiffuseStrength, iblSpecularStrength });
		return material;
	}

	std::shared_ptr<GLframework::PBRMaterial> createEngineWorldProbeMaterial()
	{
		return createPbrMaterial({ 0.25f, 0.85f, 0.42f }, 0.0f, 0.38f);
	}

	std::shared_ptr<GLframework::Mesh> createEngineWorldProbeMesh(
		const std::shared_ptr<GLframework::Renderer>& renderer
	)
	{
		if (!renderer)
		{
			return nullptr;
		}

		auto geometry = GLframework::Geometry::createSphere(
			renderer->getShader(GLframework::MaterialType::PBRMaterial),
			0.42f,
			32,
			16
		);
		auto mesh = std::make_shared<GLframework::Mesh>(geometry, createEngineWorldProbeMaterial());
		mesh->setName("Engine World Scene Probe Mesh");
		return mesh;
	}

	std::shared_ptr<GLframework::Mesh> createEngineWorldMinimalSphere(
		const std::shared_ptr<GLframework::Geometry>& geometry,
		const std::shared_ptr<GLframework::PBRMaterial>& material,
		const std::string& name
	)
	{
		auto mesh = std::make_shared<GLframework::Mesh>(geometry, material);
		mesh->setName(name);
		return mesh;
	}

	bool addMinimalMeshActor(
		GLengine::Level& level,
		GLengine::SceneComponent& rootComponent,
		const std::shared_ptr<GLframework::Mesh>& mesh,
		const std::string& actorName,
		const std::string& actorPersistentId,
		const std::string& componentPersistentId,
		const GLengine::Transform& transform,
		GL_SCENE::WorldDrivenMinimalSceneStats& stats
	)
	{
		if (!mesh)
		{
			return false;
		}

		auto& meshActor = level.spawnActor<GLengine::MeshActor>(mesh, actorName);
		meshActor.setPersistentId(actorPersistentId);
		++stats.createdActors;
		++stats.createdMeshes;

		GLengine::SceneComponent* meshComponent = meshActor.getRootComponent();
		if (!meshComponent)
		{
			return false;
		}

		meshComponent->setPersistentId(componentPersistentId);
		meshComponent->setRelativeTransform(transform);
		meshComponent->attachTo(&rootComponent);
		return true;
	}

	GLengine::Transform makeTransform(
		GLengine::Vector3 location,
		GLengine::Vector3 rotation = {},
		GLengine::Vector3 scale = { 1.0f, 1.0f, 1.0f }
	)
	{
		GLengine::Transform transform{};
		transform.location = location;
		transform.rotation = rotation;
		transform.scale = scale;
		return transform;
	}
}

GL_SCENE::WorldDrivenSceneProbeStats GL_SCENE::addEngineWorldSceneProbe(
	const std::shared_ptr<GLframework::Renderer>& renderer,
	const std::shared_ptr<GLframework::Scene>& scene
)
{
	GLengine::World world("Verification Engine World");
	return addEngineWorldSceneProbe(renderer, scene, world);
}

GL_SCENE::WorldDrivenSceneProbeStats GL_SCENE::addEngineWorldSceneProbe(
	const std::shared_ptr<GLframework::Renderer>& renderer,
	const std::shared_ptr<GLframework::Scene>& scene,
	GLengine::World& world
)
{
	WorldDrivenSceneProbeStats stats{};
	if (!renderer || !scene)
	{
		return stats;
	}

	GLengine::Level* level = world.getPersistentLevel();
	if (!level)
	{
		return stats;
	}
	world.setPersistentId(makePresetPersistentId("world", kSceneProbePreset));
	level->setPersistentId(makePresetPersistentId("level", kSceneProbePreset, { "persistent-level" }));

	auto rootObject = std::make_shared<GLframework::Object>();
	rootObject->setName("Engine World Scene Probe Root");
	auto& rootActor = level->spawnActor<GLengine::LegacyObjectActor>(
		rootObject,
		"Engine World Scene Probe Root Actor"
	);
	rootActor.setPersistentId(makePresetPersistentId("actor", kSceneProbePreset, { "root" }));
	++stats.createdActors;

	GLengine::SceneComponent* rootComponent = rootActor.getRootComponent();
	if (!rootComponent)
	{
		return stats;
	}
	rootComponent->setPersistentId(makePresetPersistentId("component", kSceneProbePreset, { "root" }));

	GLengine::Transform rootTransform{};
	rootTransform.location = { -2.15f, -1.0f, 2.05f };
	rootTransform.rotation = { 0.0f, -12.0f, 0.0f };
	rootComponent->setRelativeTransform(rootTransform);

	auto mesh = createEngineWorldProbeMesh(renderer);
	if (!mesh)
	{
		return stats;
	}

	auto& meshActor = level->spawnActor<GLengine::MeshActor>(
		mesh,
		"Engine World Scene Probe Mesh Actor"
	);
	meshActor.setPersistentId(makePresetPersistentId("actor", kSceneProbePreset, { "mesh" }));
	++stats.createdActors;
	++stats.createdMeshes;

	GLengine::SceneComponent* meshComponent = meshActor.getRootComponent();
	if (!meshComponent)
	{
		return stats;
	}
	meshComponent->setPersistentId(makePresetPersistentId("component", kSceneProbePreset, { "mesh" }));

	GLengine::Transform meshTransform{};
	meshTransform.location = { 0.0f, 0.0f, 0.0f };
	meshComponent->setRelativeTransform(meshTransform);
	meshComponent->attachTo(rootComponent);

	stats.exportStats = GLengine::WorldLegacySceneExporter::exportWorldToScene(world, scene);
	stats.added = stats.exportStats.exportedObjects > 0;
	return stats;
}

GL_SCENE::WorldDrivenMinimalSceneStats GL_SCENE::addEngineWorldMinimalScene(
	const std::shared_ptr<GLframework::Renderer>& renderer,
	const std::shared_ptr<GLframework::Scene>& scene
)
{
	GLengine::World world("Verification Minimal Engine World");
	return addEngineWorldMinimalScene(renderer, scene, world);
}

GL_SCENE::WorldDrivenMinimalSceneStats GL_SCENE::addEngineWorldMinimalScene(
	const std::shared_ptr<GLframework::Renderer>& renderer,
	const std::shared_ptr<GLframework::Scene>& scene,
	GLengine::World& world
)
{
	WorldDrivenMinimalSceneStats stats{};
	if (!renderer || !scene)
	{
		return stats;
	}

	GLengine::Level* level = world.getPersistentLevel();
	if (!level)
	{
		return stats;
	}
	world.setPersistentId(makePresetPersistentId("world", kMinimalScenePreset));
	level->setPersistentId(makePresetPersistentId("level", kMinimalScenePreset, { "persistent-level" }));

	auto rootObject = std::make_shared<GLframework::Object>();
	rootObject->setName("Engine World Minimal Root");
	auto& rootActor = level->spawnActor<GLengine::LegacyObjectActor>(
		rootObject,
		"Engine World Minimal Root Actor"
	);
	rootActor.setPersistentId(makePresetPersistentId("actor", kMinimalScenePreset, { "root" }));
	++stats.createdActors;

	GLengine::SceneComponent* rootComponent = rootActor.getRootComponent();
	if (!rootComponent)
	{
		return stats;
	}
	rootComponent->setPersistentId(makePresetPersistentId("component", kMinimalScenePreset, { "root" }));

	rootComponent->setRelativeTransform(makeTransform({ 0.0f, -0.2f, 1.55f }));

	auto sphereGeometry = GLframework::Geometry::createSphere(
		renderer->getShader(GLframework::MaterialType::PBRMaterial),
		0.36f,
		40,
		20
	);

	addMinimalMeshActor(
		*level,
		*rootComponent,
		createEngineWorldMinimalSphere(
			sphereGeometry,
			createPbrMaterial({ 0.92f, 0.38f, 0.18f }, 0.0f, 0.72f),
			"Engine World Minimal Matte Sphere"
		),
		"Engine World Minimal Matte Actor",
		makePresetPersistentId("actor", kMinimalScenePreset, { "matte" }),
		makePresetPersistentId("component", kMinimalScenePreset, { "matte" }),
		makeTransform({ -1.05f, 0.0f, 0.0f }),
		stats
	);
	addMinimalMeshActor(
		*level,
		*rootComponent,
		createEngineWorldMinimalSphere(
			sphereGeometry,
			createPbrMaterial({ 1.0f, 0.78f, 0.28f }, 1.0f, 0.22f, 1.0f, 1.25f),
			"Engine World Minimal Metallic Sphere"
		),
		"Engine World Minimal Metallic Actor",
		makePresetPersistentId("actor", kMinimalScenePreset, { "metallic" }),
		makePresetPersistentId("component", kMinimalScenePreset, { "metallic" }),
		makeTransform({ 0.0f, 0.0f, 0.0f }),
		stats
	);
	addMinimalMeshActor(
		*level,
		*rootComponent,
		createEngineWorldMinimalSphere(
			sphereGeometry,
			createPbrMaterial({ 0.16f, 0.62f, 1.0f }, 0.0f, 0.16f, 1.0f, 1.3f),
			"Engine World Minimal Gloss Sphere"
		),
		"Engine World Minimal Gloss Actor",
		makePresetPersistentId("actor", kMinimalScenePreset, { "gloss" }),
		makePresetPersistentId("component", kMinimalScenePreset, { "gloss" }),
		makeTransform({ 1.05f, 0.0f, 0.0f }),
		stats
	);
	auto emissiveMaterial = createPbrMaterial({ 0.02f, 0.02f, 0.02f }, 0.0f, 0.95f, 0.0f, 0.0f);
	emissiveMaterial->setIbl({ false, 0.0f, 0.0f });
	emissiveMaterial->setEmissiveColor({ 0.25f, 0.95f, 0.8f });
	emissiveMaterial->setEmissiveIntensity(2.4f);
	addMinimalMeshActor(
		*level,
		*rootComponent,
		createEngineWorldMinimalSphere(
			sphereGeometry,
			emissiveMaterial,
			"Engine World Minimal Emissive Sphere"
		),
		"Engine World Minimal Emissive Actor",
		makePresetPersistentId("actor", kMinimalScenePreset, { "emissive" }),
		makePresetPersistentId("component", kMinimalScenePreset, { "emissive" }),
		makeTransform({ 0.0f, 0.72f, -0.18f }, {}, { 0.55f, 0.55f, 0.55f }),
		stats
	);

	stats.exportStats = GLengine::WorldLegacySceneExporter::exportWorldToScene(world, scene);
	stats.added = stats.exportStats.exportedMeshes > 0;
	return stats;
}

std::string GL_SCENE::formatEngineWorldSceneProbeStats(const WorldDrivenSceneProbeStats& stats)
{
	const GLengine::WorldLegacySceneExportStats& exportStats = stats.exportStats;
	return "Engine world scene probe stats: actors=" + std::to_string(exportStats.visitedActors)
		+ ", sceneComponents=" + std::to_string(exportStats.visitedSceneComponents)
		+ ", exportedObjects=" + std::to_string(exportStats.exportedObjects)
		+ ", exportedMeshes=" + std::to_string(exportStats.exportedMeshes)
		+ ", exportedLights=" + std::to_string(exportStats.exportedLights)
		+ ", exportedLegacyObjects=" + std::to_string(exportStats.exportedLegacyObjects)
		+ ", sceneRootObjects=" + std::to_string(exportStats.sceneRootObjects)
		+ ", objectAttachments=" + std::to_string(exportStats.objectAttachments)
		+ ", createdActors=" + std::to_string(stats.createdActors)
		+ ", createdMeshes=" + std::to_string(stats.createdMeshes)
		+ ", added=" + std::string(stats.added ? "yes" : "no");
}

std::string GL_SCENE::formatEngineWorldMinimalSceneStats(const WorldDrivenMinimalSceneStats& stats)
{
	const GLengine::WorldLegacySceneExportStats& exportStats = stats.exportStats;
	return "Engine world minimal scene stats: actors=" + std::to_string(exportStats.visitedActors)
		+ ", sceneComponents=" + std::to_string(exportStats.visitedSceneComponents)
		+ ", exportedObjects=" + std::to_string(exportStats.exportedObjects)
		+ ", exportedMeshes=" + std::to_string(exportStats.exportedMeshes)
		+ ", exportedLights=" + std::to_string(exportStats.exportedLights)
		+ ", exportedLegacyObjects=" + std::to_string(exportStats.exportedLegacyObjects)
		+ ", sceneRootObjects=" + std::to_string(exportStats.sceneRootObjects)
		+ ", objectAttachments=" + std::to_string(exportStats.objectAttachments)
		+ ", createdActors=" + std::to_string(stats.createdActors)
		+ ", createdMeshes=" + std::to_string(stats.createdMeshes)
		+ ", added=" + std::string(stats.added ? "yes" : "no");
}
