#include "KDpch.h"
#include "XAudioEngine.h"
#include "Kaidel/Core/Application.h"
#include "Kaidel/Events/AudioEvent.h"
#include "AudioSourceCallback.h"

#include "AudioSource.h"
#include <xaudio2.h>
#include <minimp3.h>
#include <minimp3_ex.h>


#define MAKE_CHUNK_TYPE(a,b,c,d) (uint32_t)((uint32_t)a << 24) + (uint32_t)((uint32_t)b << 16) + (uint32_t)((uint32_t)c << 8) + (uint32_t)((uint32_t)d << 0)

namespace Kaidel {
	namespace Utils {
		static uint8_t ReadU8(std::ifstream& file) {
			uint8_t data = 0;
			file.read((char*)&data, sizeof(uint8_t));
			return (uint8_t)data;
		}

		static int Read16LE(std::ifstream& file)
		{
			int z = ReadU8(file);
			return z + (ReadU8(file) << 8);
		}
		static uint32_t ReadU32LE(std::ifstream& file)
		{
			uint32_t z = Read16LE(file);
			z += (uint32_t)Read16LE(file) << 16;
			return z;
		}

		static int Read16BE(std::ifstream& file)
		{
			int z = ReadU8(file);
			return (z << 8) + ReadU8(file);
		}
		static uint32_t ReadU32BE(std::ifstream& file)
		{
			uint32_t z = Read16BE(file);
			return (z << 16) + Read16BE(file);
		}

		static void LoadWaveFile(std::ifstream& file, WAVEFORMATEX& wfx, uint8_t*& data, uint64_t& size) {
			ZeroMemory(&wfx, sizeof(wfx));

			while (file) {
				uint32_t chunkType = Utils::ReadU32BE(file);

				std::string_view type((const char*)&chunkType, 4);
				KD_CORE_INFO("{}", type);
				switch (chunkType)
				{
				case 'RIFF':
				{
					uint32_t chunkSize = Utils::ReadU32LE(file);
					uint32_t waveSize = chunkSize - 4;

					uint32_t waveID = Utils::ReadU32BE(file);
					KD_CORE_ASSERT(waveID == 'WAVE');
				} break;
				case 'fmt ':
				{
					uint32_t chunkSize = Utils::ReadU32LE(file);

					wfx.wFormatTag = Utils::Read16LE(file);
					wfx.nChannels = Utils::Read16LE(file);
					wfx.nSamplesPerSec = Utils::ReadU32LE(file);
					wfx.nAvgBytesPerSec = Utils::ReadU32LE(file);
					wfx.nBlockAlign = Utils::Read16LE(file);
					wfx.wBitsPerSample = Utils::Read16LE(file);

					//if (chunkSize >= 18) {
					//	wfx.Format.cbSize = Utils::Read16LE(file);
					//}
					//
					//if (chunkSize == 40) {
					//	wfx.Samples.wValidBitsPerSample = Utils::Read16LE(file);
					//	wfx.dwChannelMask = Utils::ReadU32BE(file);
					//	Utils::ReadU32LE(file);
					//	Utils::ReadU32LE(file);
					//	Utils::ReadU32LE(file);
					//	Utils::ReadU32LE(file);
					//}

					KD_CORE_ASSERT(wfx.wFormatTag == 1);
				} break;
				case 'data':
				{
					uint32_t chunkSize = Utils::ReadU32LE(file);
					data = new uint8_t[chunkSize];
					size = chunkSize;
					file.read((char*)data, chunkSize);
					if ((chunkSize & 2) == 1) {
						Utils::ReadU8(file);
					}
				} break;
				default:
				{
					uint32_t chunkSize = Utils::ReadU32LE(file);
					file.seekg(chunkSize, std::ios::cur);
				} break;
				}
			}

			wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
			wfx.cbSize = 0;
		}

		static void LoadMP3File(const std::string& file, WAVEFORMATEX& wfx, uint8_t*& data, uint64_t& size) {
			mp3dec_t* decoder = new mp3dec_t;
			mp3dec_init(decoder);

			mp3dec_file_info_t info;
			KD_CORE_ASSERT(mp3dec_load(decoder, file.c_str(), &info, nullptr, nullptr) == 0);

			ZeroMemory(&wfx, sizeof(wfx));

			wfx.wFormatTag = WAVE_FORMAT_PCM;
			wfx.nChannels = info.channels;
			wfx.nSamplesPerSec = info.hz;
			wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nChannels * sizeof(int16_t);
			wfx.nBlockAlign = wfx.nChannels * sizeof(int16_t);
			wfx.wBitsPerSample = 16;

			size = info.samples * sizeof(int16_t);
			data = new uint8_t[size];
			std::memcpy(data, info.buffer, size);

			delete decoder;
		}

		/*
		#include <mpg123.h>  
#include <xaudio2.h>  
#include <iostream>  
#include <fstream>  
#include <vector>  
#include <thread>  
#include <atomic>  

// Audio streaming class  
class MP3AudioStream {  
public:  
    MP3AudioStream(const std::string& filename)  
        : m_running(true), m_position(0), m_filename(filename)   
    {  
        mpg123_init();  
        m_mh = mpg123_new(NULL, NULL);  
        if (m_mh == NULL || mpg123_open(m_mh, filename.c_str()) != MPG123_OK) {  
            std::cerr << "Failed to open MP3 file." << std::endl;  
            throw std::runtime_error("MP3 file initialization failed");  
        }  

        mpg123_getformat(m_mh, &m_waveFormat.nRate, &m_waveFormat.nChannels, &m_waveFormat.wFormatTag);  
        m_waveFormat.wBitsPerSample = 16;  
        m_waveFormat.nBlockAlign = m_waveFormat.nChannels * (m_waveFormat.wBitsPerSample / 8);  
        m_waveFormat.nAvgBytesPerSec = m_waveFormat.nRate * m_waveFormat.nBlockAlign;  
        
        // Start the streaming thread  
        m_streamThread = std::thread(&MP3AudioStream::StreamAudio, this);  
    }  

    ~MP3AudioStream() {  
        m_running = false; // Signal the thread to stop  
        if (m_streamThread.joinable()) {  
            m_streamThread.join();  
        }  
        mpg123_close(m_mh);  
        mpg123_delete(m_mh);  
        mpg123_exit();  
    }  

    void StreamAudio() {  
        while (m_running) {  
            if (m_position >= BUFFER_COUNT) {  
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Avoid busy-wait  
                continue;  
            }  

            BYTE audioBuffer[BUFFER_SIZE];  
            size_t done;  
            int ret = mpg123_read(m_mh, audioBuffer, BUFFER_SIZE, &done);  
            if (ret == MPG123_OK || ret == MPG123_DONE) {  
                // Submit buffer to XAudio2 (assumes pSourceVoice and your setup exists)  
                XAUDIO2_BUFFER buffer = {};  
                buffer.AudioBytes = static_cast<UINT32>(done);  
                buffer.pAudioData = audioBuffer;  
                buffer.Flags = XAUDIO2_END_OF_STREAM; // Or manage this flag based on your logic  
                pSourceVoice->SubmitSourceBuffer(&buffer);  
                m_position++;  
            } else {  
                if (ret == MPG123_DONE) {  
                    // Handle end of stream (stop or loop)  
                    m_running = false;  
                }  
            }  
        }  
    }  

private:  
    static const int BUFFER_SIZE = 32768;  // Size of audio buffer (32 KB)  
    static const int BUFFER_COUNT = 4;      // Number of buffers to queue  
    std::atomic<bool> m_running;  
    size_t m_position;  
    std::string m_filename;  
    mpg123_handle *m_mh;  
    WAVEFORMATEX m_waveFormat;  
    std::thread m_streamThread;  
};  

// Usage example  
int main() {  
    IXAudio2* pXAudio2;  
    IXAudio2MasteringVoice* pMasterVoice;  

    HRESULT hr = XAudio2Create(&pXAudio2);  
    if (SUCCEEDED(hr)) {  
        pXAudio2->CreateMasteringVoice(&pMasterVoice);  
        
        // Create the streaming audio object  
        try {  
            MP3AudioStream mp3Stream("example.mp3");  
            // The streaming will happen in the background  

            // Simulate game loop or main thread processing  
            while (mp3Stream.IsPlaying()) {  
                std::this_thread::sleep_for(std::chrono::milliseconds(100));  
            }  
        } catch (const std::exception& e) {  
            std::cerr << "Error: " << e.what() << std::endl;  
        }  
    }  

    // Clean-up  
    if (pMasterVoice) pMasterVoice->DestroyVoice();  
    if (pXAudio2) pXAudio2->Release();  

    return 0;  
}
*/
	}

	struct XAudioEngineData {
		IXAudio2* AudioEngine = nullptr;
		IXAudio2MasteringVoice* OutputDevice = nullptr;
	}* s_XAudioEngineData;

    void XAudioEngine::Init()
    {
		s_XAudioEngineData = new XAudioEngineData;
		KD_CORE_ASSERT(XAudio2Create(&s_XAudioEngineData->AudioEngine, 0, XAUDIO2_USE_DEFAULT_PROCESSOR) == S_OK);
		s_XAudioEngineData->AudioEngine->CreateMasteringVoice(&s_XAudioEngineData->OutputDevice);
    }
    void XAudioEngine::Shutdown()
    {
		s_XAudioEngineData->OutputDevice->DestroyVoice();
		delete s_XAudioEngineData->AudioEngine;
    }

	Ref<AudioSource> XAudioEngine::CreateAudioSource(const std::string& path)
	{
		auto extension = FileSystem::path(path).extension();

		Ref<AudioSource> output = CreateRef<AudioSource>();

		if (extension == ".wav") {
			std::ifstream file(path, std::ios::binary);
			KD_CORE_ASSERT(file.is_open());

			Utils::LoadWaveFile(file, output->m_Format, output->m_BufferData, output->m_BufferSize);
		}
		else if (extension == ".mp3")
			Utils::LoadMP3File(path, output->m_Format, output->m_BufferData,output->m_BufferSize);

		output->m_EventsCallback = new AudioSourceCallback(output);

		KD_CORE_ASSERT(
			s_XAudioEngineData->AudioEngine->CreateSourceVoice(
				&output->m_SourceVoice, &output->m_Format, 0,
				XAUDIO2_DEFAULT_FREQ_RATIO, output->m_EventsCallback, 
				NULL, NULL) 
		== S_OK);

		
		output->m_SourceVoice->SetVolume(1.0f);

		return output;
	}
	void XAudioEngine::Play(Ref<AudioSource> source, const std::chrono::duration<double>& duration)
	{
		double totalSeconds = duration.count();

		UINT32 startSampleIndex = totalSeconds * source->m_Format.nSamplesPerSec;

		size_t startByteIndex = startSampleIndex * source->m_Format.nChannels * sizeof(int16_t);

		XAUDIO2_BUFFER buffer = {};
		buffer.AudioBytes = (source->m_BufferSize - startByteIndex);
		buffer.pAudioData = source->m_BufferData + startByteIndex;
		buffer.Flags = XAUDIO2_END_OF_STREAM;

		if (source->m_BufferSize <= startByteIndex)
			return;

		KD_CORE_ASSERT(source->m_SourceVoice->SubmitSourceBuffer(&buffer) == S_OK);
		source->m_SourceVoice->Start();
	}
	void XAudioEngine::Stop(Ref<AudioSource> source)
	{
		source->m_SourceVoice->Stop();
		source->m_SourceVoice->FlushSourceBuffers();
	}

	bool XAudioEngine::IsPlaying(Ref<AudioSource> source)
	{
		XAUDIO2_VOICE_STATE state;
		source->m_SourceVoice->GetState(&state, 0);
		return state.BuffersQueued != 0;
	}
}
