#pragma once
#include "Entity.h"
namespace Kaidel {

	enum class PrimitiveType {
		None,Rect,Circle
	};


	template<PrimitiveType pType>
	void CreatePrimitive(Entity entity) {
		switch (pType)
		{
		case Kaidel::PrimitiveType::Rect:
		{
			entity.AddComponent<SpriteRendererComponent>();
			entity.GetComponent<TagComponent>().Tag = "Square";
			break;
		}
		case Kaidel::PrimitiveType::Circle:
		{
			entity.AddComponent<CircleRendererComponent>();
			entity.GetComponent<TagComponent>().Tag = "Circle";
			break;
		}
		default:
			break;
		}
		KD_ERROR("CreatePrimitive not implemented for {}", pType);
	}
	


}
