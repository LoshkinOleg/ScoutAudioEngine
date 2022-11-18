#pragma once

#include <chrono>
#include <memory>

#include <Scout/TypedefAndEnum.h>

namespace Scout
{
	class IAudioEngine
	{
	public:
		virtual void PlaySound(const SoundHandle sound) = 0;
		virtual void PlayOneShot(const SoundHandle sound) = 0;
		virtual void StopSound(const SoundHandle sound) = 0;

		virtual void PauseSound(const SoundHandle sound) = 0;
		virtual void UnpauseSound(const SoundHandle sound) = 0;

		virtual void SetSoundLooped(const SoundHandle sound, const bool newVal) = 0;

		virtual bool IsSoundPlaying(const SoundHandle sound) const = 0;
		virtual bool IsSoundPaused(const SoundHandle sound) const = 0;
		virtual bool IsSoundLooped(const SoundHandle sound) const = 0;

		virtual Bitdepth GetBitdepth() const = 0;
		virtual Samplerate GetSamplerate() const = 0;
		virtual std::uint64_t GetBytesPerFrame() const = 0;
		virtual std::uint64_t GetNrOfChannels() const = 0;
		virtual std::uint64_t GetFramesPerBuffer() const = 0;
		virtual std::uint64_t GetBufferSizeInBytes() const = 0;
		virtual std::chrono::milliseconds GetBufferLatency() const = 0;

		virtual SoundHandle MakeSound(
			const std::vector<float>& data, const std::uint64_t nrOfChannels,
			const bool interleaved = true) = 0;

		virtual void Update() = 0;
	};

    struct AudioEngineDef
    {
        AudioApi implementation = AudioApi::NONE;
		Bitdepth engineBufferQuantisation = Bitdepth::F32;
		Samplerate engineBufferSamplerate = Samplerate::Hz_48k;
		SpeakerSetup speakersSetup = SpeakerSetup::MONO;
		std::chrono::milliseconds desiredLatency = std::chrono::milliseconds((std::uint64_t)(1.0 / 30.0 * 1000.0)); // 30 Hz
    };

    /*
        Factory function for creating audio renderer implementations.
    */
    std::unique_ptr<IAudioEngine> MakeAudioEngine(const AudioEngineDef def);
}