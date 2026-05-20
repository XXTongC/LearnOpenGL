#include "ShadowMeshDraw.h"

#include "renderer/MeshDraw.h"

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

bool ShadowMeshDraw::draw(const std::shared_ptr<Mesh>& mesh)
{
	return MeshDraw::drawIndexed(mesh);
}
