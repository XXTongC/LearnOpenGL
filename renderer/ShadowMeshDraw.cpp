#include "ShadowMeshDraw.h"

#include "mesh/instancedMesh.h"

using namespace GLframework;

bool ShadowMeshDraw::isPostProcessPass(const std::vector<std::shared_ptr<Mesh>>& meshes)
{
	for (const auto& mesh : meshes)
	{
		const auto material = mesh ? mesh->getMaterial() : nullptr;
		if (!material || material->getMaterialType() != MaterialType::ScreenMaterial)
		{
			return false;
		}
	}

	return true;
}

void ShadowMeshDraw::draw(const std::shared_ptr<Mesh>& mesh)
{
	auto geometry = mesh->getGeometry();
	glBindVertexArray(geometry->getVao());

	if (mesh->getType() == ObjectType::InstancedMesh)
	{
		const auto im = std::static_pointer_cast<InstancedMesh>(mesh);
		glDrawElementsInstanced(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr, im->getInstanceCount());
	}
	else
	{
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);
}
