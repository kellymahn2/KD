#pragma once
#include "Kaidel/Core/Base.h"
#include "AudioSource.h"

#include <string>

namespace Kaidel {
	class AudioSource;

	class XAudioEngine {
	public:
		static void Init();
		
		static void Shutdown();

		static Ref<AudioSource> CreateAudioSource(const std::string& path);

		static void Play(Ref<AudioSource> source, const std::chrono::duration<double>& duration = std::chrono::duration<double>(0));

		static void Stop(Ref<AudioSource> source);

		static bool IsPlaying(Ref<AudioSource> source);
	};
}
