#pragma once

#include <cstdint>
#include <stdexcept>

#include <portaudio.h>

#include <Scout/TypedefAndEnum.h>

namespace Scout
{
	static constexpr const std::uint32_t END_OF_DATA = (std::uint32_t)-1;

	enum class Samplerate_Portaudio: std::uint32_t
	{
		NONE = 0,

		Hz_8k = 8000,
		Hz_16k = 16000,
		Hz_22050 = 22050,
		Hz_44100 = 44100,
		Hz_48k = 48000,
		Hz_96k = 96000,
		Hz_384k = 384000,
	};

	enum class Bitdepth_Portaudio: std::uint32_t
	{
		NONE = 0,

		F32 = paFloat32,
	};

	enum class SpeakerSetup_Portaudio: std::uint32_t
	{
		NONE = 0,

		// 1 channel
		MONO,

		// 2 channels
		DUAL_MONO,
	};

	inline Samplerate_Portaudio ToImplEnum_Portaudio(const Samplerate rate)
	{
		switch(rate)
		{
			case Samplerate::NONE: return Samplerate_Portaudio::NONE;
			case Samplerate::Hz_8k: return Samplerate_Portaudio::Hz_8k;
			case Samplerate::Hz_16k: return Samplerate_Portaudio::Hz_16k;
			case Samplerate::Hz_22050: return Samplerate_Portaudio::Hz_22050;
			case Samplerate::Hz_44100: return Samplerate_Portaudio::Hz_44100;
			case Samplerate::Hz_48k: return Samplerate_Portaudio::Hz_48k;
			case Samplerate::Hz_96k: return Samplerate_Portaudio::Hz_96k;
			case Samplerate::Hz_384k: return Samplerate_Portaudio::Hz_384k;

			default:
			{
				throw std::runtime_error("Cannot convert specified Samplerate to Portaudio implementation enum.");
			}break;
		}
	}

	inline Samplerate ToAbstractEnum_Portaudio(const Samplerate_Portaudio rate)
	{
		switch(rate)
		{
			case Scout::Samplerate_Portaudio::NONE: return Scout::Samplerate::NONE;
			case Scout::Samplerate_Portaudio::Hz_8k: return Scout::Samplerate::Hz_8k;
			case Scout::Samplerate_Portaudio::Hz_16k: return Scout::Samplerate::Hz_16k;
			case Scout::Samplerate_Portaudio::Hz_22050: return Scout::Samplerate::Hz_22050;
			case Scout::Samplerate_Portaudio::Hz_44100: return Scout::Samplerate::Hz_44100;
			case Scout::Samplerate_Portaudio::Hz_48k: return Scout::Samplerate::Hz_48k;
			case Scout::Samplerate_Portaudio::Hz_96k: return Scout::Samplerate::Hz_96k;
			case Scout::Samplerate_Portaudio::Hz_384k: return Scout::Samplerate::Hz_384k;

			default:
			{
				throw std::runtime_error("Failed to convert implementation enum to abstract enum.");
			}
			break;
		}
	}

	inline Bitdepth_Portaudio ToImplEnum_Portaudio(const Bitdepth depth)
	{
		switch(depth)
		{
			case Bitdepth::NONE: return Bitdepth_Portaudio::NONE;
			case Bitdepth::F32: return Bitdepth_Portaudio::F32;

			default:
			{
				throw std::runtime_error("Cannot convert specified Bitdepth to Portaudio implementation enum.");
			}break;
		}
	}

	inline Bitdepth ToAbstractEnum_Portaudio(const Bitdepth_Portaudio depth)
	{
		switch(depth)
		{
			case Scout::Bitdepth_Portaudio::NONE: return Scout::Bitdepth::NONE;
			case Scout::Bitdepth_Portaudio::F32: return Scout::Bitdepth::F32;

			default:
			{
				throw std::runtime_error("Failed to convert implementation enum to abstract enum.");
			}
			break;
		}
	}

	inline SpeakerSetup_Portaudio ToImplEnum_Portaudio(const SpeakerSetup setup)
	{
		switch(setup)
		{
			case SpeakerSetup::NONE: return SpeakerSetup_Portaudio::NONE;
			case SpeakerSetup::MONO: return SpeakerSetup_Portaudio::MONO;
			case SpeakerSetup::DUAL_MONO: return SpeakerSetup_Portaudio::DUAL_MONO;

			default:
			{
				throw std::runtime_error("Cannot convert specified speaker setup to Portaudio implementation enum.");
			}break;
		}
	}
}