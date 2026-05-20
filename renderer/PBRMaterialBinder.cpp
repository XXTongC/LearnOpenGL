#include "PBRMaterialBinder.h"

#include "renderer/EnvironmentRenderTargets.h"
#include "renderer/LightResourceBinder.h"
#include "renderer/ShadowResourceBinder.h"

using namespace GLframework;

namespace
{
	void setMVPMatrices(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh, Camera* camera)
	{
		shader->setMat4("modelMatrix", mesh->getModelMatrix());
		shader->setMat4("viewMatrix", camera->getViewMatrix());
		shader->setMat4("projectionMatrix", camera->getProjectionMatrix());
	}

	void setNormalMatrix(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Mesh>& mesh)
	{
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(mesh->getModelMatrix()))));
	}

	void setCommonMaterialUniforms(const std::shared_ptr<Shader>& shader, const std::shared_ptr<PBRMaterial>& material, Camera* camera)
	{
		shader->setFloat("opacity", material->getOpacity());
		shader->setFloat("time", static_cast<float>(glfwGetTime()));
		shader->setFloat("speed", 0.5f);
		shader->setVector3("cameraPosition", camera->mPosition);
	}

	void bindTexture(const std::shared_ptr<Shader>& shader, const char* samplerName, const std::shared_ptr<Texture>& texture)
	{
		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}

	void bindOptionalTexture(
		const std::shared_ptr<Shader>& shader,
		const char* samplerName,
		const char* useFlagName,
		const std::shared_ptr<Texture>& texture
	)
	{
		shader->setInt(useFlagName, texture != nullptr ? 1 : 0);
		if (!texture)
		{
			return;
		}

		shader->setInt(samplerName, texture->getUnit());
		texture->Bind();
	}

	bool canUseIBL(const std::shared_ptr<PBRMaterial>& material, const EnvironmentRenderTargets* environmentTargets)
	{
		return material->mUseIBL
			&& environmentTargets != nullptr
			&& environmentTargets->isInitialized()
			&& environmentTargets->hasPrecomputedEnvironment();
	}

	void bindPBRSurfaceUniforms(const std::shared_ptr<Shader>& shader, const std::shared_ptr<PBRMaterial>& material)
	{
		for (const auto& slot : material->getVec3UniformSlots())
		{
			shader->setVector3(slot.uniformName, slot.value ? *slot.value : glm::vec3{ 0.0f });
		}

		for (const auto& slot : material->getSurfaceFloatUniformSlots())
		{
			shader->setFloat(slot.uniformName, slot.value ? *slot.value : 0.0f);
		}

		for (const auto& slot : material->getTextureSlots())
		{
			bindOptionalTexture(
				shader,
				slot.samplerUniform,
				slot.useFlagUniform,
				slot.texture ? *slot.texture : nullptr
			);
		}
	}

	void bindPBRIBLUniforms(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<PBRMaterial>& material,
		const EnvironmentRenderTargets* environmentTargets,
		bool useIBL
	)
	{
		shader->setInt("useIBL", useIBL ? 1 : 0);
		for (const auto& slot : material->getIblFloatUniformSlots())
		{
			shader->setFloat(slot.uniformName, slot.value ? *slot.value : 0.0f);
		}

		if (!useIBL)
		{
			return;
		}

		const unsigned int maxMipLevels = environmentTargets->getMaxPrefilterMipLevels();
		shader->setFloat("iblMaxReflectionLod", maxMipLevels > 0 ? static_cast<float>(maxMipLevels - 1) : 0.0f);
		bindTexture(shader, "irradianceMap", environmentTargets->getIrradianceMap());
		bindTexture(shader, "prefilterMap", environmentTargets->getPrefilterMap());
		bindTexture(shader, "brdfLut", environmentTargets->getBrdfLut());
	}
}

bool PBRMaterialBinder::bind(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<PBRMaterial>& material,
	const std::shared_ptr<Mesh>& mesh,
	Camera* camera,
	const std::shared_ptr<DirectionalLight>& dirLight,
	const std::shared_ptr<SpotLight>& spotLight,
	const std::vector<std::shared_ptr<PointLight>>& pointLights,
	const std::shared_ptr<AmbientLight>& ambient,
	const EnvironmentRenderTargets* environmentTargets
)
{
	if (!shader || !material || !mesh || camera == nullptr)
	{
		return false;
	}

	const bool useIBL = canUseIBL(material, environmentTargets);
	setCommonMaterialUniforms(shader, material, camera);
	setMVPMatrices(shader, mesh, camera);
	setNormalMatrix(shader, mesh);
	LightResourceBinder::bindForwardLights(shader, dirLight, spotLight, pointLights, ambient);
	ShadowResourceBinder::bindCSMShadowResources(shader, camera, dirLight, 8);
	bindPBRSurfaceUniforms(shader, material);
	bindPBRIBLUniforms(shader, material, environmentTargets, useIBL);
	return true;
}
