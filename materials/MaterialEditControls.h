#pragma once

#include <memory>

#include "../third_party/glm/glm.hpp"

namespace GLframework
{
	class Texture;

	struct PhongSurfaceEditControls
	{
		float* shininess{ nullptr };
		const std::shared_ptr<Texture>* diffuseTexture{ nullptr };
		const std::shared_ptr<Texture>* specularMaskTexture{ nullptr };
	};

	struct PhongSurfaceInput
	{
		std::shared_ptr<Texture> diffuseTexture{ nullptr };
		std::shared_ptr<Texture> specularMaskTexture{ nullptr };
		float shininess{ 10.0f };
	};

	struct PhongSurfaceRuntimeState
	{
		const std::shared_ptr<Texture>* diffuseTexture{ nullptr };
		const std::shared_ptr<Texture>* specularMaskTexture{ nullptr };
		float shininess{ 10.0f };
	};

	struct GrassMaterialEditControls
	{
		float* shininess{ nullptr };
		float* uvScale{ nullptr };
		float* brightness{ nullptr };
		float* windScale{ nullptr };
		float* phaseScale{ nullptr };
		glm::vec3* windDirection{ nullptr };
		float* cloudLerp{ nullptr };
		float* cloudUvScale{ nullptr };
		float* cloudSpeed{ nullptr };
		glm::vec3* cloudWhiteColor{ nullptr };
		glm::vec3* cloudBlackColor{ nullptr };
		const std::shared_ptr<Texture>* diffuseTexture{ nullptr };
		const std::shared_ptr<Texture>* opacityMaskTexture{ nullptr };
		const std::shared_ptr<Texture>* cloudMaskTexture{ nullptr };
	};

	struct ScreenMaterialInputTextures
	{
		const std::shared_ptr<Texture>* screenTexture{ nullptr };
		const std::shared_ptr<Texture>* bloomTexture{ nullptr };
		const std::shared_ptr<Texture>* depthStencilTexture{ nullptr };
	};

	struct PBRMaterialEditControls
	{
		bool* useAlphaMask{ nullptr };
		float* alphaCutoff{ nullptr };
		int* metallicMapChannel{ nullptr };
		int* roughnessMapChannel{ nullptr };
		int* aoMapChannel{ nullptr };
		bool* useIbl{ nullptr };
	};
}
