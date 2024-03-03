#pragma once

#include "Event.h"
#include "Kaidel/Renderer/Settings.h"

namespace Kaidel {
	class SettingsEvent : public Event {
	public:
		EVENT_CLASS_CATEGORY(EventCategory::EventCategorySettings)

	};

	class RendererSettingsChangedEvent : public SettingsEvent {
	public:
		
		EVENT_CLASS_TYPE(RendererSettingsChanged)

		std::string ToString() const override
		{
			return "Renderer Settings Changed";
		}
	};

}
