#include "EditorContext.h"

#include <variant>

namespace Kaidel {

	struct SelectedAnimationData
	{
		AnimationFrame* SelectedAnimationFrame;
		AnimationValueType SelectedAnimationValueType;
	};

	struct EditorContextData
	{
		std::variant<Entity, SelectedAnimationData> Data;
	};

	static EditorContextData* s_Data;

	void EditorContext::Init()
	{
		s_Data = new EditorContextData;
	}

	void EditorContext::Shutdown()
	{
		delete s_Data;
	}

	Entity EditorContext::SelectedEntity()
	{
		Entity* entity = std::get_if<0>(&s_Data->Data);
		if (entity)
			return *entity;
		return {};
	}


	void EditorContext::SelectedEntity(Entity selectedEntity)
	{
		s_Data->Data = selectedEntity;
	}

	AnimationFrame* EditorContext::SelectedAnimationFrame()
	{
		SelectedAnimationData* data = std::get_if<1>(&s_Data->Data);
		if (data)
			return data->SelectedAnimationFrame;
		return nullptr;
	}

	AnimationValueType EditorContext::SelectedAnimationFrameValueType()
	{
		SelectedAnimationData* data = std::get_if<1>(&s_Data->Data);
		if (data)
			return data->SelectedAnimationValueType;
		return AnimationValueType::MaxValue;
	}

	void EditorContext::SelectedAnimationFrame(AnimationFrame* selectedAnimationFrame, AnimationValueType type)
	{
		s_Data->Data = SelectedAnimationData{ selectedAnimationFrame, type };
	}
}
