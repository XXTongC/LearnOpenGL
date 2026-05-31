#pragma once

#include <memory>
#include <string>

#include "WorldDrivenSceneStats.h"

namespace GLengine
{
	class World;
}

namespace GLframework
{
	class Renderer;
	class Scene;
}

namespace GL_SCENE
{
	WorldDrivenSceneProbeStats addEngineWorldSceneProbe(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		const std::shared_ptr<GLframework::Scene>& scene
	);

	WorldDrivenSceneProbeStats addEngineWorldSceneProbe(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		const std::shared_ptr<GLframework::Scene>& scene,
		GLengine::World& world
	);

	WorldDrivenMinimalSceneStats addEngineWorldMinimalScene(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		const std::shared_ptr<GLframework::Scene>& scene
	);

	WorldDrivenMinimalSceneStats addEngineWorldMinimalScene(
		const std::shared_ptr<GLframework::Renderer>& renderer,
		const std::shared_ptr<GLframework::Scene>& scene,
		GLengine::World& world
	);

	std::string formatEngineWorldSceneProbeStats(const WorldDrivenSceneProbeStats& stats);
	std::string formatEngineWorldMinimalSceneStats(const WorldDrivenMinimalSceneStats& stats);
}
