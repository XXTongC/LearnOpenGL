#pragma once

#include <memory>
#include <vector>

#include "camera/camera.h"
#include "framework/scene.h"
#include "mesh/mesh.h"

namespace GLframework
{
	class RenderQueue
	{
	public:
		void build(const std::shared_ptr<Scene>& scene, Camera* camera);
		void clear();

		const std::vector<std::shared_ptr<Mesh>>& getOpacityObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getTransparentObjects() const;

	private:
		void projectObject(const std::shared_ptr<Object>& object);
		void sortTransparentObjects(Camera* camera);

		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;
	};
}
