#pragma once
#include "core.h"

namespace GLframework
{
	enum class ObjectType
	{
		Object,
		Mesh,
		Scene,
		InstancedMesh,
	};

	class Object : public std::enable_shared_from_this<Object>
	{
	public:
		
		Object(glm::vec3 position = glm::vec3{0.0f}, float anglex= 0.0f, float angley = 0.0f, float anglez = 0.0f, glm::vec3 scale =glm::vec3{1.0f},ObjectType object = ObjectType::Object)
			:mPosition(position),mAngleX(anglex),mAngleY(angley),mAngleZ(anglez),mScale(scale),mType(object)
		{}
		virtual ~Object()
		{
			std::cout << "object destroy" << std::endl;
		}
		std::shared_ptr<Object> getShared() { return shared_from_this(); }
		//parent and children
		void addChild (	std::shared_ptr<Object> obj);
		void addParent(	std::shared_ptr<Object> obj);
		std::shared_ptr<Object>						getParent();
		std::vector<std::shared_ptr<Object>>		getChildren();

		//setType
		ObjectType getType() const;
		void setType(ObjectType type);

		void setPosition(glm::vec3 pos);
		glm::vec3 getPosition() const;
		//增量旋转
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);

		//设置旋转角
		void setAngleX(float angle);
		void setAngleY(float angle);
		void setAngleZ(float angle);


		void setScale(glm::vec3 scale);

		glm::mat4 getModelMatrix() const;
		glm::vec3 getDirection() const;
	protected:
		glm::vec3 mPosition{ 0.0f };
		float mAngleX{ 0.0f };
		float mAngleY{ 0.0f };
		float mAngleZ{ 0.0f };
		glm::vec3 mScale{ 1.0f };
		std::vector<std::shared_ptr<Object>>	mChildren;
		std::shared_ptr<Object>					mParent = nullptr;
		//类型记录
		ObjectType mType;
	};
}