#pragma once

#include <memory>
#include <vector>

class Camera;

namespace GLframework
{
	class Mesh;
	class Object;
	class Scene;

	class RenderQueue
	{
	public:
		void build(const std::shared_ptr<Scene>& scene, Camera* camera);
		void clear();

		const std::vector<std::shared_ptr<Mesh>>& getOpacityObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getTransparentObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getLegacyOpacityObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getLegacyTransparentObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getPbrOpacityObjects() const;
		const std::vector<std::shared_ptr<Mesh>>& getPbrTransparentObjects() const;

	private:
		void projectObject(const std::shared_ptr<Object>& object);
		void sortTransparentObjects(Camera* camera, std::vector<std::shared_ptr<Mesh>>& objects);

		std::vector<std::shared_ptr<Mesh>> mOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mTransparentObjects;
		std::vector<std::shared_ptr<Mesh>> mLegacyOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mLegacyTransparentObjects;
		std::vector<std::shared_ptr<Mesh>> mPbrOpacityObjects;
		std::vector<std::shared_ptr<Mesh>> mPbrTransparentObjects;
	};
}
