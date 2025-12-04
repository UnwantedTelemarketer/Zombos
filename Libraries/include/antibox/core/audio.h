#pragma once

#include "miniaudio/miniaudio.h"
#include <map>
#include <string>
#include <vector>

struct AudioModifier {
	float pitch = 1.f;
	float volume = 1.f;
};

class AudioEngine {
private:
	ma_engine mEngine;
	std::map<std::string, ma_device*> mDevices;
	std::map<std::string, ma_decoder*> mDecoders;
	ma_result mResult;
	ma_device_config mDeviceConfig;
	bool isPlaying1;
public:
	ma_result init();
	void PlayAudio(const char* path);
	void PlayAudioLooping(std::string, std::string name);
	void PlayModAudio(const char* path, AudioModifier mod);
	void StopAudioLooping(std::string name);
	void SetVolume(float volume);
	void SetVolumeLoop(float volume, std::string name);
	float GetVolume();
};
