#include <Scout/IAudioEngine.h>

#include <Scout/Portaudio/AudioEngine.h>

namespace Scout
{
	std::unique_ptr<IAudioEngine> MakeAudioEngine(const AudioEngineDef def)
	{
		if(def.implementation != AudioApi::PORTAUDIO) throw std::runtime_error("Currently only supporting Portaudio implementation.");

		return std::make_unique<AudioEngine_Portaudio>(
			ToImplEnum_Portaudio(def.engineBufferQuantisation),
			ToImplEnum_Portaudio(def.engineBufferSamplerate),
			ToImplEnum_Portaudio(def.speakersSetup),
			def.desiredLatency);
	}
}