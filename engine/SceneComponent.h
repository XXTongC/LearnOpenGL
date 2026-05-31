#pragma once

#include <vector>

#include "engine/ActorComponent.h"
#include "engine/Transform.h"

namespace GLengine
{
	class SceneComponent : public ActorComponent
	{
	public:
		explicit SceneComponent(std::string name = {});
		~SceneComponent() override;

		const Transform& getRelativeTransform() const;
		void setRelativeTransform(const Transform& transform);

		SceneComponent* getParent() const;
		const std::vector<SceneComponent*>& getChildren() const;
		void attachTo(SceneComponent* parent);
		void detachFromParent();

	private:
		Transform mRelativeTransform{};
		SceneComponent* mParent{ nullptr };
		std::vector<SceneComponent*> mChildren{};
	};
}
