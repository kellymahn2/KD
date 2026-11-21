#pragma once

#include "Event.h"

#include "Kaidel/Scene/Entity.h"


namespace Kaidel
{
	class EntityEvent : public Event
	{
	public:

		Entity GetEntity() const { return m_Entity; }

		EVENT_CLASS_CATEGORY(EventCategoryEntity);
	protected:
		EntityEvent(Entity entity)
			: m_Entity(entity) {
		}

		Entity m_Entity;
	};

	class EntityCreatedEvent : public EntityEvent
	{
	public:
		EntityCreatedEvent(Entity entity) : EntityEvent(entity) {}

		EVENT_CLASS_TYPE(EntityCreated);

		std::string ToString() const override
		{
			return "EntityCreatedEvent";
		}
	};

	class EntityRemovedEvent : public EntityEvent
	{
	public:
		EntityRemovedEvent(Entity entity) : EntityEvent(entity) {}

		EVENT_CLASS_TYPE(EntityRemoved);

		std::string ToString() const override
		{
			return "EntityRemovedEvent";
		}
	};
}
