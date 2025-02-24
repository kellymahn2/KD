#pragma once

#include "KDpch.h"

#include <xaudio2.h>

namespace Kaidel {
	class AudioSource : public IRCCounter<false> {
	public:
		AudioSource() = default;
		~AudioSource();
	private:
		WAVEFORMATEX m_Format;
		IXAudio2SourceVoice* m_SourceVoice = nullptr;
		uint8_t* m_BufferData = nullptr;
		uint64_t m_BufferSize = 0;
		//XAUDIO2_BUFFER m_Buffer;
		IXAudio2VoiceCallback* m_EventsCallback = nullptr;

		friend class XAudioEngine;
	};
}
