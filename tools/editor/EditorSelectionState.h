#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../third_party/glm/glm.hpp"

class Camera;

namespace GLengine
{
	class Actor;
	class ActorComponent;
	class SceneComponent;
}

namespace GLframework
{
	class Object;
	class Shadow;
}

namespace GL_EDITOR
{
	enum class EditTransactionRecordKind
	{
		TransformVec3,
		Lifecycle
	};

	struct EditTransactionRecord
	{
		EditTransactionRecordKind kind{ EditTransactionRecordKind::TransformVec3 };
		std::uint64_t sequence{ 0 };
		std::string targetLabel{};
		std::string targetType{};
		std::uint64_t targetObjectId{ 0 };
		std::uintptr_t targetAddress{ 0 };
		GLengine::SceneComponent* sceneComponent{ nullptr };
		std::string field{};
		glm::vec3 beforeValue{};
		glm::vec3 afterValue{};
	};

	class EditTransactionLog
	{
	public:
		void recordVec3(
			std::string targetLabel,
			std::string targetType,
			std::uint64_t targetObjectId,
			std::uintptr_t targetAddress,
			GLengine::SceneComponent* sceneComponent,
			std::string field,
			glm::vec3 beforeValue,
			glm::vec3 afterValue
		)
		{
			EditTransactionRecord record{};
			record.kind = EditTransactionRecordKind::TransformVec3;
			record.sequence = mNextSequence++;
			record.targetLabel = std::move(targetLabel);
			record.targetType = std::move(targetType);
			record.targetObjectId = targetObjectId;
			record.targetAddress = targetAddress;
			record.sceneComponent = sceneComponent;
			record.field = std::move(field);
			record.beforeValue = beforeValue;
			record.afterValue = afterValue;
			mRecords.push_back(std::move(record));
			mDirty = true;
		}

		void recordLifecycle(
			std::string targetLabel,
			std::string targetType,
			std::uint64_t targetObjectId,
			std::uintptr_t targetAddress,
			std::string field
		)
		{
			EditTransactionRecord record{};
			record.kind = EditTransactionRecordKind::Lifecycle;
			record.sequence = mNextSequence++;
			record.targetLabel = std::move(targetLabel);
			record.targetType = std::move(targetType);
			record.targetObjectId = targetObjectId;
			record.targetAddress = targetAddress;
			record.field = std::move(field);
			mRecords.push_back(std::move(record));
			mDirty = true;
		}

		const std::vector<EditTransactionRecord>& getRecords() const
		{
			return mRecords;
		}

		const EditTransactionRecord* getLatestRecord() const
		{
			return mRecords.empty() ? nullptr : &mRecords.back();
		}

		EditTransactionRecord popLatestRecord()
		{
			EditTransactionRecord record{};
			if (!mRecords.empty())
			{
				record = mRecords.back();
				mRecords.pop_back();
			}
			mDirty = !mRecords.empty();
			return record;
		}

		void markDirty()
		{
			mDirty = true;
		}

		void markSaved()
		{
			mDirty = false;
		}

		void clear()
		{
			mRecords.clear();
			mDirty = false;
		}

		bool isDirty() const
		{
			return mDirty;
		}

	private:
		std::vector<EditTransactionRecord> mRecords{};
		std::uint64_t mNextSequence{ 1 };
		bool mDirty{ false };
	};

	enum class SelectionKind
	{
		None,
		Object,
		Camera,
		Shadow,
		Actor,
		Component,
		Asset,
	};

	struct SelectionContext
	{
		SelectionKind kind{ SelectionKind::None };
		std::weak_ptr<GLframework::Object> selectedObject{};
		std::weak_ptr<GLframework::Shadow> selectedShadow{};
		Camera* selectedCamera{ nullptr };
		GLengine::Actor* selectedActor{ nullptr };
		GLengine::ActorComponent* selectedComponent{ nullptr };
		std::string selectedAssetHandle{};
		std::string label{};
	};

	void ensureSelectionIsInitialized(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& defaultObject);
	std::shared_ptr<GLframework::Object> getSelectedObject(const SelectionContext& selection);
	std::shared_ptr<GLframework::Shadow> getSelectedShadow(const SelectionContext& selection);
	Camera* getSelectedCamera(const SelectionContext& selection);
	GLengine::Actor* getSelectedActor(const SelectionContext& selection);
	GLengine::ActorComponent* getSelectedComponent(const SelectionContext& selection);
	const std::string& getSelectedAssetHandle(const SelectionContext& selection);

	void selectObject(SelectionContext& selection, const std::shared_ptr<GLframework::Object>& object);
	void selectShadow(SelectionContext& selection, const std::shared_ptr<GLframework::Shadow>& shadow, const std::string& label);
	void selectCamera(SelectionContext& selection, Camera* selectedCamera, const std::string& label);
	void selectActor(SelectionContext& selection, GLengine::Actor* actor, const std::string& label);
	void selectComponent(SelectionContext& selection, GLengine::ActorComponent* component, const std::string& label);
	void selectAsset(SelectionContext& selection, std::string assetHandle, const std::string& label);
}
