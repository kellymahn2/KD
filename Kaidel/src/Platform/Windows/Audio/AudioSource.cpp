#include "KDpch.h"
#include "AudioSource.h"
#include "AudioSourceCallback.h"

#include "XAudioEngine.h"


namespace Kaidel {
	AudioSource::~AudioSource()
	{
		m_SourceVoice->Stop();
		m_SourceVoice->DestroyVoice();
		delete m_EventsCallback;
		delete[] m_BufferData;
	}
}
