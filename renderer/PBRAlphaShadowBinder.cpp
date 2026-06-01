#include "PBRAlphaShadowBinder.h"

#include "framework/shader.h"
#include "materials/pbrMaterial/PBRMaterial.h"
#include "mesh/mesh.h"

using namespace GLframework;

namespace
{
	std::shared_ptr<PBRMaterial> getAlphaMaskedMaterial(const std::shared_ptr<Mesh>& mesh)
	{
		const auto material = mesh ? mesh->getMaterial() : nullptr;
		if (!material || material->getMaterialType() != MaterialType::PBRMaterial)
		{
			return nullptr;
		}

		auto pbrMaterial = std::static_pointer_cast<PBRMaterial>(material);
		return pbrMaterial->alphaMaskState().useAlphaMask ? pbrMaterial : nullptr;
	}

	void bindAlphaMaskUniforms(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<PBRMaterial>& material
	)
	{
		const auto alphaMask = material->alphaMaskState();
		const auto textures = material->textureState();
		const auto albedoMap = textures.albedoMap ? *textures.albedoMap : nullptr;
		shader->setFloat("alphaCutoff", alphaMask.alphaCutoff);
		shader->setInt("useAlbedoMap", albedoMap ? 1 : 0);
		if (!albedoMap)
		{
			return;
		}

		shader->setInt("albedoMap", albedoMap->getUnit());
		albedoMap->Bind();
	}
}

bool PBRAlphaShadowBinder::isAlphaMaskedPbrMesh(const std::shared_ptr<Mesh>& mesh)
{
	return getAlphaMaskedMaterial(mesh) != nullptr;
}

bool PBRAlphaShadowBinder::bindDirectional(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<Mesh>& mesh,
	const glm::mat4& lightMatrix
)
{
	const auto material = getAlphaMaskedMaterial(mesh);
	if (!shader || !mesh || !material)
	{
		return false;
	}

	shader->setMat4("lightMatrix", lightMatrix);
	shader->setMat4("modelMatrix", mesh->getModelMatrix());
	bindAlphaMaskUniforms(shader, material);
	return true;
}

bool PBRAlphaShadowBinder::bindPoint(
	const std::shared_ptr<Shader>& shader,
	const std::shared_ptr<Mesh>& mesh,
	const glm::mat4& lightSpaceMatrix,
	const glm::vec3& lightPosition,
	float farPlane
)
{
	const auto material = getAlphaMaskedMaterial(mesh);
	if (!shader || !mesh || !material)
	{
		return false;
	}

	shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
	shader->setMat4("modelMatrix", mesh->getModelMatrix());
	shader->setVector3("lightPos", lightPosition);
	shader->setFloat("far_plane", farPlane);
	bindAlphaMaskUniforms(shader, material);
	return true;
}
