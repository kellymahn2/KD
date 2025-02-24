#pragma once

#include "Event.h"

#include "Platform/Windows/Audio/AudioSource.h"

namespace Kaidel {
	
	class AudioStartEvent : public Event {
	public:
		AudioStartEvent(Ref<AudioSource> source)
			:m_Source(std::move(source)) {}
		
		Ref<AudioSource> GetSource() { return m_Source; }

		std::string ToString() const override
		{
			return fmt::format("AudioStartEvent: {}", (const void*)m_Source.Get());
		}

		EVENT_CLASS_TYPE(AudioStart)
		EVENT_CLASS_CATEGORY(EventCategoryAudio)
	private:
		Ref<AudioSource> m_Source;
	};

	class AudioEndEvent : public Event {
	public:
		AudioEndEvent(Ref<AudioSource> source)
			:m_Source(std::move(source)) {}

		Ref<AudioSource> GetSource() { return m_Source; }

		std::string ToString() const override
		{
			return fmt::format("AudioEndEvent: {}", (const void*)m_Source.Get());
		}

		EVENT_CLASS_TYPE(AudioEnd)
		EVENT_CLASS_CATEGORY(EventCategoryAudio)
	private:
		Ref<AudioSource> m_Source;
	};

	class AudioLoopEndEvent : public Event {
	public:
		AudioLoopEndEvent(Ref<AudioSource> source)
			:m_Source(std::move(source)) {}

		Ref<AudioSource> GetSource() { return m_Source; }

		std::string ToString() const override
		{
			return fmt::format("LoopEndEvent: {}", (const void*)m_Source.Get());
		}

		EVENT_CLASS_TYPE(AudioLoopEnd)
		EVENT_CLASS_CATEGORY(EventCategoryAudio)
	private:
		Ref<AudioSource> m_Source;
	};
}
