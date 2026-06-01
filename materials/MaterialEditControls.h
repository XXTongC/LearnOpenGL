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

	struct GrassSurfaceInput
	{
		std::shared_ptr<Texture> diffuseTexture{ nullptr };
		std::shared_ptr<Texture> specularMaskTexture{ nullptr };
		std::shared_ptr<Texture> opacityMaskTexture{ nullptr };
		std::shared_ptr<Texture> cloudMaskTexture{ nullptr };
		float shininess{ 10.0f };
	};

	struct GrassSurfaceRuntimeState
	{
		const std::shared_ptr<Texture>* diffuseTexture{ nullptr };
		const std::shared_ptr<Texture>* specularMaskTexture{ nullptr };
		const std::shared_ptr<Texture>* opacityMaskTexture{ nullptr };
		const std::shared_ptr<Texture>* cloudMaskTexture{ nullptr };
		float shininess{ 10.0f };
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

	struct PBRSurfaceInput
	{
		glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
		glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
		float metallic{ 0.0f };
		float roughness{ 0.5f };
		float ao{ 1.0f };
		float emissiveIntensity{ 0.0f };
	};

	struct PBRSurfaceRuntimeState
	{
		glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
		glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
		float metallic{ 0.0f };
		float roughness{ 0.5f };
		float ao{ 1.0f };
		float emissiveIntensity{ 0.0f };
	};

	struct PBRTextureInput
	{
		std::shared_ptr<Texture> albedoMap{ nullptr };
		std::shared_ptr<Texture> metallicMap{ nullptr };
		std::shared_ptr<Texture> roughnessMap{ nullptr };
		std::shared_ptr<Texture> aoMap{ nullptr };
		std::shared_ptr<Texture> normalMap{ nullptr };
		std::shared_ptr<Texture> emissiveMap{ nullptr };
	};

	struct PBRTextureRuntimeState
	{
		const std::shared_ptr<Texture>* albedoMap{ nullptr };
		const std::shared_ptr<Texture>* metallicMap{ nullptr };
		const std::shared_ptr<Texture>* roughnessMap{ nullptr };
		const std::shared_ptr<Texture>* aoMap{ nullptr };
		const std::shared_ptr<Texture>* normalMap{ nullptr };
		const std::shared_ptr<Texture>* emissiveMap{ nullptr };
	};

	struct PBRTextureChannelInput
	{
		int metallicMapChannel{ 0 };
		int roughnessMapChannel{ 0 };
		int aoMapChannel{ 0 };
	};

	struct PBRTextureChannelRuntimeState
	{
		int metallicMapChannel{ 0 };
		int roughnessMapChannel{ 0 };
		int aoMapChannel{ 0 };
	};

	struct PBRAlphaMaskInput
	{
		bool useAlphaMask{ false };
		float alphaCutoff{ 0.5f };
	};

	struct PBRAlphaMaskRuntimeState
	{
		bool useAlphaMask{ false };
		float alphaCutoff{ 0.5f };
	};

	struct PBRIblInput
	{
		bool useIbl{ false };
		float diffuseStrength{ 1.0f };
		float specularStrength{ 1.0f };
	};

	struct PBRIblRuntimeState
	{
		bool useIbl{ false };
		float diffuseStrength{ 1.0f };
		float specularStrength{ 1.0f };
	};
}
