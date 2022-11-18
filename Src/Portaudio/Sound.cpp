#include <Scout/Portaudio/Sound.h>

#include <cstring>

namespace Scout
{
	MonoSound_Portaudio::MonoSound_Portaudio(const std::vector<float>& data):
		data(data)
	{}
	void MonoSound_Portaudio::AdvanceBy(const std::uint32_t frames)
	{
		if(currentBegin == END_OF_DATA) return;

		const std::uint32_t max = (std::uint32_t)data.size();
		const std::uint32_t nextBegin = currentBegin + frames;

		if (loop)
		{
			if (nextBegin < max) // if not wrapping around audio data
			{
				currentBegin = nextBegin;
			}
			else
			{
				currentBegin = nextBegin - max;
			}
		}
		else
		{
			if (nextBegin <= max) // if not wrapping around audio data
			{
				currentBegin = nextBegin;
			}
			else
			{
				currentBegin = END_OF_DATA;
			}
		}
	}
	void MonoSound_Portaudio::GoToFrame(const std::uint32_t frame)
	{
		if(frame >= data.size()) throw std::runtime_error("Trying to go to a frame beyond sound data.");

		currentBegin = frame;
	}
	void MonoSound_Portaudio::Service(std::vector<float>& outBuff) const
	{
		const auto buffSize = (std::uint32_t)outBuff.size();
		const auto max = (std::uint32_t)data.size();
		
		std::uint32_t currentEnd = currentBegin + buffSize;
		if (currentEnd >= max) // if wrapping around audio data
		{
			currentEnd = currentEnd - max;
		}

		if (currentEnd > currentBegin) // not wrapping around buffer of audio data
		{
			std::copy(data.begin() + currentBegin, data.begin() + currentEnd, outBuff.begin());
		}
		else
		{
			const auto firstPartDelta = max - currentBegin;
			const auto secondPartDelta = currentEnd;
			std::copy(data.begin() + currentBegin, data.end(), outBuff.begin());
			std::copy(data.begin(), data.begin() + secondPartDelta, outBuff.begin() + firstPartDelta);
		}
	}

	void MonoSound_Portaudio::Play()
	{
		currentBegin = 0;
	}
	void MonoSound_Portaudio::Stop()
	{
		currentBegin = END_OF_DATA;
	}
}