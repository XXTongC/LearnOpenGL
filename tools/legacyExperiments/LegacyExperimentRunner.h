#pragma once

#include <cstddef>
#include <memory>
#include <vector>

namespace GLframework
{
	class DirectionalLight;
	class GrassInstanceMaterial;
	class Mesh;
	class Object;
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
		void enableSolarSystem(RuntimeContext& context);
		void enableGrassField(RuntimeContext& context, int rowCount, int columnCount);
		void enableEnvironmentSphere(RuntimeContext& context);
		void enableCsmPlane(RuntimeContext& context);
		void enableBackpackModel(RuntimeContext& context);
		void enableShadowPreview(RuntimeContext& context);
		void enableOrbitingPointLight(std::size_t lightIndex = 0, float radius = 3.0f, float height = 3.0f);
		void update(RuntimeContext& context);

	private:
		struct SolarSystemState
		{
			bool enabled{ false };
			float speed{ 0.01f };
			std::shared_ptr<GLframework::Object> roundForEarth{ nullptr };
			std::shared_ptr<GLframework::Object> roundForVenus{ nullptr };
			std::shared_ptr<GLframework::Object> roundForUranus{ nullptr };
			std::shared_ptr<GLframework::Object> roundForSaturn{ nullptr };
			std::shared_ptr<GLframework::Object> roundForNeptune{ nullptr };
			std::shared_ptr<GLframework::Object> roundForJupiter{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMars{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMercury{ nullptr };
			std::shared_ptr<GLframework::Object> roundForMoon{ nullptr };
		};

		struct OrbitingPointLightState
		{
			bool enabled{ false };
			std::size_t lightIndex{ 0 };
			float radius{ 3.0f };
			float height{ 3.0f };
		};

		void updateSolarSystem();
		void updateOrbitingPointLight(RuntimeContext& context);

		SolarSystemState mSolarSystem{};
		OrbitingPointLightState mOrbitingPointLight{};
		bool mGrassFieldEnabled{ false };
		bool mEnvironmentSphereEnabled{ false };
		bool mCsmPlaneEnabled{ false };
		bool mBackpackEnabled{ false };
		bool mShadowPreviewEnabled{ false };
	};
}
