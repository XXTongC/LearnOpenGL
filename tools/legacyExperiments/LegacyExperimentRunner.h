#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace GLframework
{
	class DirectionalLight;
	class GrassInstanceMaterial;
	class Mesh;
	class PhongCSMShadowMaterial;
	class PointLight;
	class Renderer;
	class Scene;
}

namespace GL_EXPERIMENTS
{
	struct RuntimeContext
	{
		std::shared_ptr<GLframework::Renderer>& renderer;
		std::shared_ptr<GLframework::Scene>& sceneOffScreen;
		std::shared_ptr<GLframework::GrassInstanceMaterial>& grassMaterial;
		std::shared_ptr<GLframework::Mesh>& skyBoxMesh;
		std::shared_ptr<GLframework::Mesh>& movePlane;
		std::shared_ptr<GLframework::PhongCSMShadowMaterial>& csmShadowMaterial;
		std::shared_ptr<GLframework::DirectionalLight>& dirLight;
		std::vector<std::shared_ptr<GLframework::PointLight>>& pointLights;
	};

	class LegacyExperimentRunner
	{
	public:
		LegacyExperimentRunner();
		~LegacyExperimentRunner();

		LegacyExperimentRunner(const LegacyExperimentRunner&) = delete;
		LegacyExperimentRunner& operator=(const LegacyExperimentRunner&) = delete;
		LegacyExperimentRunner(LegacyExperimentRunner&&) noexcept;
		LegacyExperimentRunner& operator=(LegacyExperimentRunner&&) noexcept;

		void enableSolarSystem(RuntimeContext& context);
		void enableGrassField(RuntimeContext& context, int rowCount, int columnCount);
		void enableEnvironmentSphere(RuntimeContext& context);
		void enableCsmPlane(RuntimeContext& context);
		void enableBackpackModel(RuntimeContext& context);
		void enableShadowPreview(RuntimeContext& context);
		void enableOrbitingPointLight(std::size_t lightIndex = 0, float radius = 3.0f, float height = 3.0f);
		void update(RuntimeContext& context);

	private:
		void updateSolarSystem();
		void updateOrbitingPointLight(RuntimeContext& context);

		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}
