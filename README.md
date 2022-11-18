# Scout Audio Engine
My own barebones audio engine capable of playing back loaded PCM audio.

# TODO
- Implement support for DSP
- Write an FMod implementation
- Write a Wwise implementation
- Write a 3DTI implementation
- Implement support for (actual) stereo in portaudio implementation
- Implement support for >2 channel audio
- Implement audio data streaming

# BUG
- Bugged on linux: accessing AudioEngine_Portaudio::buffer_ in AudioEngine_Portaudio::ServicePortaudio_ results in a segmentation fault.
