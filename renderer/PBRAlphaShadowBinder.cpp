#include "PBRAlphaShadowBinder.h"

#include "materials/pbrMaterial/PBRMaterial.h"

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
		return pbrMaterial->mUseAlphaMask ? pbrMaterial : nullptr;
	}

	void bindAlphaMaskUniforms(
		const std::shared_ptr<Shader>& shader,
		const std::shared_ptr<PBRMaterial>& material
	)
	{
		shader->setFloat("alphaCutoff", material->mAlphaCutoff);
		shader->setInt("useAlbedoMap", material->mAlbedoMap ? 1 : 0);
		if (!material->mAlbedoMap)
		{
			return;
		}

		shader->setInt("albedoMap", material->mAlbedoMap->getUnit());
		material->mAlbedoMap->Bind();
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
