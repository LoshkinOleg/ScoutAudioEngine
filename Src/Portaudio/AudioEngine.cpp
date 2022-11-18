#include <Scout/Portaudio/AudioEngine.h>

#include <iostream>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <cassert>

#include <Scout/Math.h>
#include <Scout/AudioMixing.h>

namespace Scout
{
	AudioEngine_Portaudio::AudioEngine_Portaudio(
		const Bitdepth_Portaudio quant, const Samplerate_Portaudio sampleRate,
		const SpeakerSetup_Portaudio speakersSetup, const std::chrono::milliseconds desiredLatency):
		quant_(quant), sampleRate_(sampleRate), speakerSetup_(speakersSetup)
	{
		const double sampleDuration = 1.0 / (double)((std::uint32_t)sampleRate_);
		const double latencyDuration = (double)desiredLatency.count() * 0.001;
		framesPerBuffer_ = (std::uint32_t)NearestUpperPowOfTwo((std::uint64_t)(latencyDuration / sampleDuration));

		int nrOfChannels;
		switch(speakerSetup_)
		{
			case Scout::SpeakerSetup_Portaudio::MONO:
			{
				nrOfChannels = 1;
			}break;

			case Scout::SpeakerSetup_Portaudio::DUAL_MONO:
			{
				nrOfChannels = 2;
			}break;

			default:
			{
				throw std::runtime_error("Unexpected speakerSetup_ .");
			}break;
		}

		auto err = Pa_Initialize();
		if(err != paNoError) throw std::runtime_error(std::string("Failed to initialize PortAudio: ") + Pa_GetErrorText(err));

		selectedDevice_ = Pa_GetDefaultOutputDevice();
		if(selectedDevice_ == paNoDevice) throw std::runtime_error(std::string("PortAudio failed to retireve a default playback device."));

		PaStreamParameters outputParams{
			selectedDevice_,
			nrOfChannels,
			(std::uint32_t)quant_,
			latencyDuration,
			NULL
		};

		err = Pa_OpenStream(
			&pStream_,
			NULL,
			&outputParams,
			(double)sampleRate,
			(unsigned int)framesPerBuffer_,
			paNoFlag,
			&ServicePortaudio_,
			this); // Bug on linux: Try replacing this with some ad-hoc struct to see if problem persists?
		if(err != paNoError) throw std::runtime_error(std::string("Failed to open a stream to default playback device: ") + Pa_GetErrorText(err));

		err = Pa_StartStream(pStream_);
		if(err != paNoError) throw std::runtime_error(std::string("Failed to start stream to default playback device: ") + Pa_GetErrorText(err));

		buffer_.resize((std::uint64_t)(framesPerBuffer_ * (std::uint32_t)nrOfChannels), 0.0f);
	}

	AudioEngine_Portaudio::~AudioEngine_Portaudio()
	{
		auto err = Pa_StopStream(pStream_);
		if(err != paNoError) std::cerr << std::string("Error stopping stream to playback device: ") + Pa_GetErrorText(err) << std::endl;
		err = Pa_CloseStream(pStream_);
		if(err != paNoError) std::cerr << std::string("Error closing stream to playback device: ") + Pa_GetErrorText(err) << std::endl;
		err = Pa_Terminate();
		if(err != paNoError) std::cerr << std::string("Error shutting down PortAudio: ") + Pa_GetErrorText(err) << std::endl;
	}

	SoundHandle AudioEngine_Portaudio::MakeSound(const std::vector<float>& data, const std::uint64_t nrOfChannels, const bool interleaved)
	{
		// TODO: take into consideration sample rate: resample source data to internal sample rate?
		// TODO: create multichannel sound classes rather than just getting the first channel of a multichannel audio signal

		if (nrOfChannels > 1)
		{
			if (interleaved)
			{
				std::vector<float> signal = data;
				UninterleaveSignal(signal, nrOfChannels);
				sounds_.push_back(std::vector<float>(signal.begin(), signal.begin() + data.size() / nrOfChannels));
				return sounds_.size() - 1;
			}
			else
			{
				sounds_.push_back(std::vector<float>(data.begin(), data.begin() + data.size() / nrOfChannels));
				return sounds_.size() - 1;
			}
		}

		sounds_.push_back(std::vector<float>(data.begin(), data.end()));
		return sounds_.size() - 1;
	}

	void AudioEngine_Portaudio::PlaySound(const SoundHandle sound)
	{
		sounds_[sound].Play();
		playing_.insert(sound);
	}

	void AudioEngine_Portaudio::PlayOneShot(const SoundHandle sound)
	{
		sounds_[sound].Play();
		sounds_[sound].loop = false;
		playing_.insert(sound);
	}

	void AudioEngine_Portaudio::StopSound(const SoundHandle sound)
	{
		sounds_[sound].Stop();
		playing_.erase(sound);
	}

	void AudioEngine_Portaudio::StopAll()
	{
		for(auto& sound : sounds_)
		{
			sound.Stop();
		}
		playing_.clear();
	}

	void AudioEngine_Portaudio::PauseSound(const SoundHandle sound)
	{
		playing_.erase(sound);
	}

	void AudioEngine_Portaudio::UnpauseSound(const SoundHandle sound)
	{
		playing_.insert(sound);
	}

	void AudioEngine_Portaudio::SetSoundLooping(const SoundHandle sound, const bool newLooping)
	{
		sounds_[sound].loop = newLooping;
	}

	bool AudioEngine_Portaudio::IsSoundPlaying(const SoundHandle sound) const
	{
		// Sound is considered to be playing if it's currentBegin index is not pointing to END_OF_DATA.
		return sounds_[sound].currentBegin != END_OF_DATA;
	}

	bool AudioEngine_Portaudio::AudioEngine_Portaudio::IsSoundPaused(const SoundHandle sound) const
	{
		// Sound is considered to be paused if it's currently "playing" but not is not part of the playing_ set .
		return IsSoundPlaying(sound) && (playing_.find(sound) == playing_.end());
	}

	bool AudioEngine_Portaudio::IsSoundLooped(const SoundHandle sound) const
	{
		return sounds_[sound].loop;
	}

	Bitdepth AudioEngine_Portaudio::GetBitdepth() const
	{
		return ToAbstractEnum_Portaudio(quant_);
	}

	Samplerate AudioEngine_Portaudio::GetSamplerate() const
	{
		return ToAbstractEnum_Portaudio(sampleRate_);
	}

	std::uint64_t AudioEngine_Portaudio::GetBytesPerFrame() const
	{
		switch(quant_)
		{
			case Scout::Bitdepth_Portaudio::F32: return 4 * GetNrOfChannels();

			default:
			{
				throw std::runtime_error("Cannot compute nr of bytes per frame due to inability to deduce size indicated by quant_ .");
			}break;
		}
	}

	std::uint64_t AudioEngine_Portaudio::GetNrOfChannels() const
	{
		switch(speakerSetup_)
		{
			case SpeakerSetup_Portaudio::MONO: return 1;
			case SpeakerSetup_Portaudio::DUAL_MONO: return 2;

			default:
			{
				throw std::runtime_error("Unexpected speakerSetup_ .");
			}break;
		}
	}

	std::uint64_t AudioEngine_Portaudio::GetFramesPerBuffer() const
	{
		return framesPerBuffer_;
	}

	std::uint64_t AudioEngine_Portaudio::GetBufferSizeInBytes() const
	{
		return framesPerBuffer_ * GetBytesPerFrame();
	}

	std::chrono::milliseconds AudioEngine_Portaudio::GetBufferLatency() const
	{
		std::uint64_t latencyInSeconds = 0;
		const auto* info = Pa_GetStreamInfo(pStream_);
		if (info)
		{
			latencyInSeconds = (std::uint64_t)info->outputLatency;
		}
		else
		{
			throw std::runtime_error("AudioEngine_Portaudio::GetBufferLatency: Failed to retireve portaudio output latency.");
		}
		return std::chrono::milliseconds(latencyInSeconds * 1000);
	}

	void AudioEngine_Portaudio::Update()
	{
		std::scoped_lock lck(bufferMutex_);
		if (!update_) return;

		static std::vector<float> outputBuff(GetBufferSizeInBytes() / sizeof(float), 0.0f);
		std::fill(outputBuff.begin(), outputBuff.end(), 0.0f);

		switch(speakerSetup_)
		{
			case Scout::SpeakerSetup_Portaudio::MONO:
			{
				const std::uint32_t framesPerBuff = (std::uint32_t)GetFramesPerBuffer();

				static std::vector<float> sumBuff(framesPerBuff, 0.0f);
				static std::vector<float> workingBuff(framesPerBuff, 0.0f);
				std::fill(sumBuff.begin(), sumBuff.end(), 0.0f);
				std::fill(workingBuff.begin(), workingBuff.end(), 0.0f);

				for(auto& handle : playing_)
				{
					sounds_[handle].Service(workingBuff);
					sounds_[handle].AdvanceBy(framesPerBuff);
					MixSignalsInPlace(sumBuff, workingBuff, MixingPolicy::SUM_AND_CLAMP);
				}

				assert(outputBuff.size() == sumBuff.size());
				std::copy(sumBuff.begin(), sumBuff.end(), outputBuff.begin());
			}break;

			case Scout::SpeakerSetup_Portaudio::DUAL_MONO:
			{
				const std::uint32_t framesPerBuff = (std::uint32_t)GetFramesPerBuffer();

				static std::vector<float> sumBuff(framesPerBuff, 0.0f);
				static std::vector<float> workingBuff(framesPerBuff, 0.0f);
				std::fill(sumBuff.begin(), sumBuff.end(), 0.0f);
				std::fill(workingBuff.begin(), workingBuff.end(), 0.0f);

				for(auto& handle : playing_)
				{
					sounds_[handle].Service(workingBuff);
					sounds_[handle].AdvanceBy(framesPerBuff);
					MixSignalsInPlace(sumBuff, workingBuff, MixingPolicy::SUM_AND_CLAMP);
				}

				assert(outputBuff.size() == 2 * sumBuff.size());
				std::copy(sumBuff.begin(), sumBuff.end(), outputBuff.begin());
				std::copy(sumBuff.begin(), sumBuff.end(), outputBuff.begin() + sumBuff.size());
				InterleaveSignal(outputBuff, 2);
			}break;

			default:
			{
				throw std::runtime_error("Specified speaker configuration isn't handled by this implementation.");
			}break;
		}

		// Copy to next buffer to present.
		std::copy(outputBuff.begin(), outputBuff.end(), buffer_.begin());
		update_ = false;
	}

	int AudioEngine_Portaudio::ServicePortaudio_(const void* input, void* output,
								 unsigned long frameCount,
								 const PaStreamCallbackTimeInfo* timeInfo,
								 PaStreamCallbackFlags statusFlags,
								 void* userData)
	{
		AudioEngine_Portaudio& self = *static_cast<AudioEngine_Portaudio*>(userData);
		std::scoped_lock lck(self.bufferMutex_);

		try
		{
			// BUG: trying to access self.buffer_ on linux results in a segmentation fault.
			std::memcpy(output, self.buffer_.data(), frameCount * self.GetNrOfChannels() * sizeof(float));
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error(std::string("AudioEngine_Portaudio::ServicePortaudio_: Encountered error servicing audio: ") + e.what());
		}
		
		self.update_ = true;
		return paContinue;
	}

	void AudioEngine_Portaudio::SetSoundLooped(const SoundHandle sound, const bool newVal)
	{
		sounds_[sound].loop = newVal;
	}
}