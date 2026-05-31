#include "SceneSetupPipeline.h"

#include "../../engine/Engine.h"
#include "../../engine/LegacySceneWorldBuilder.h"
#include "../../engine/World.h"

namespace
{
	bool shouldMirrorLegacySceneToEngineWorld(const GL_SCENE::SceneSetupPipelineConfig& config)
	{
		return config.mirrorLegacySceneToEngineWorld
			&& config.useLegacyDefaultScene
			&& !config.useWorldDrivenMinimalScene
			&& !config.addWorldDrivenProbe;
	}

	GLengine::World* createRuntimeWorld(
		GL_SCENE::SetupContext& context,
		const std::string& worldName,
		bool editable
	)
	{
		if (!context.engine)
		{
			context.engineWorld = nullptr;
			context.engineWorldEditable = false;
			return nullptr;
		}

		GLengine::World& world = context.engine->createWorld(worldName);
		context.engineWorld = &world;
		context.engineWorldEditable = editable;
		return &world;
	}

	GLengine::LegacySceneImportStats mirrorLegacySceneToEngineWorld(GL_SCENE::SetupContext& context)
	{
		GLengine::World* world = createRuntimeWorld(context, "Runtime Legacy Scene Mirror", false);
		if (!world)
		{
			return {};
		}

		return GLengine::LegacySceneWorldBuilder::importScene(*world, context.sceneOffScreen);
	}

	GLengine::World* prepareRuntimeWorld(
		GL_SCENE::SetupContext& context,
		const GL_SCENE::SceneSetupPipelineConfig& config
	)
	{
		if (!config.useWorldDrivenMinimalScene && !config.addWorldDrivenProbe)
		{
			return context.engineWorld;
		}

		const std::string worldName = config.useWorldDrivenMinimalScene
			? "Runtime World Driven Minimal Scene"
			: "Runtime World Driven Scene Probe";
		return createRuntimeWorld(context, worldName, true);
	}
}

GL_SCENE::SceneSetupPipelineResult GL_SCENE::prepareScene(
	SetupContext& context,
	const SceneSetupPipelineConfig& config
)
{
	SceneSetupPipelineResult result{};
	context.engineWorld = nullptr;
	context.engineWorldEditable = false;

	if (config.useLegacyDefaultScene)
	{
		prepareDefaultScene(context);
		result.legacyDefaultPrepared = true;
		if (shouldMirrorLegacySceneToEngineWorld(config))
		{
			result.legacyWorldMirrorPrepared = true;
			result.legacyWorldMirrorStats = mirrorLegacySceneToEngineWorld(context);
		}
	}
	else
	{
		prepareSceneInfrastructure(context);
		context.engineWorldEditable = false;
	}

	GLengine::World* runtimeWorld = prepareRuntimeWorld(context, config);
	if (config.useWorldDrivenMinimalScene)
	{
		result.worldDrivenMinimalScenePrepared = true;
		if (runtimeWorld)
		{
			result.worldDrivenMinimalSceneStats = addEngineWorldMinimalScene(
				context.renderer,
				context.sceneOffScreen,
				*runtimeWorld
			);
		}
	}

	if (config.addWorldDrivenProbe)
	{
		result.worldDrivenProbeRequested = true;
		if (runtimeWorld)
		{
			result.worldDrivenProbeStats = addEngineWorldSceneProbe(
				context.renderer,
				context.sceneOffScreen,
				*runtimeWorld
			);
		}
	}

	return result;
}

std::string GL_SCENE::formatLegacyWorldMirrorStats(const GLengine::LegacySceneImportStats& stats)
{
	return "Engine world legacy mirror stats: visitedObjects=" + std::to_string(stats.visitedObjects)
		+ ", actors=" + std::to_string(stats.importedActorCount())
		+ ", meshActors=" + std::to_string(stats.meshActors)
		+ ", lightActors=" + std::to_string(stats.lightActors)
		+ ", legacyObjectActors=" + std::to_string(stats.legacyObjectActors)
		+ ", componentAttachments=" + std::to_string(stats.componentAttachments)
		+ ", persistentActors=" + std::to_string(stats.actorsWithPersistentIds)
		+ ", persistentSceneComponents=" + std::to_string(stats.sceneComponentsWithPersistentIds)
		+ ", persistentIdSource=legacy-mirror";
}
