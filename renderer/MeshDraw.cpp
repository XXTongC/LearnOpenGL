#include "MeshDraw.h"

#include "mesh/instancedMesh.h"

using namespace GLframework;

bool MeshDraw::drawIndexed(const std::shared_ptr<Mesh>& mesh)
{
	if (!mesh || !mesh->getGeometry())
	{
		return false;
	}

	const auto geometry = mesh->getGeometry();
	glBindVertexArray(geometry->getVao());

	if (mesh->getType() == ObjectType::InstancedMesh)
	{
		const auto instancedMesh = std::static_pointer_cast<InstancedMesh>(mesh);
		glDrawElementsInstanced(
			GL_TRIANGLES,
			geometry->getIndicesCount(),
			GL_UNSIGNED_INT,
			nullptr,
			instancedMesh->getInstanceCount()
		);
	}
	else
	{
		glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_INT, nullptr);
	}

	glBindVertexArray(0);
	return true;
}
