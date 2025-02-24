#pragma once
#include "Kaidel/Core/Application.h"
#include "Kaidel/Events/AudioEvent.h"

#include <xaudio2.h>

namespace Kaidel {
	class AudioSourceCallback : public IXAudio2VoiceCallback
	{
	public:
		AudioSourceCallback(Ref<AudioSource> source)
			: m_Source(source) {
		}

		HANDLE GetEvent()const { return m_Event; }

		void STDMETHODCALLTYPE OnBufferStart(void*) override 
		{ 
			Application::Get().SubmitToMainThread([source = m_Source]() {
				Application::Get().PublishEvent<AudioStartEvent>(source);
			});
		}
		void STDMETHODCALLTYPE OnLoopEnd(void*) override
		{
			Application::Get().SubmitToMainThread([source = m_Source]() {
				Application::Get().PublishEvent<AudioLoopEndEvent>(source);
			});
		}
		void STDMETHODCALLTYPE OnStreamEnd() override 
		{ 
			Application::Get().SubmitToMainThread([this]() {
				Application::Get().PublishEvent<AudioEndEvent>(m_Source);
				if (m_Source->Count == 1) {
					m_Source = {};
				}
			});
		}

		void STDMETHODCALLTYPE OnBufferEnd(void*) override {}
		void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
		void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
	private:
		template<typename T>
		void Submit() {
			Application::Get().SubmitToMainThread([source = m_Source]() {
				Application::Get().PublishEvent<T>(source);
			});
		}
	private:
		HANDLE m_Event;
		Ref<AudioSource> m_Source;
	};
}
