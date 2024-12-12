#pragma once

#include "miniaudio/miniaudio.h"
#include <map>
#include <string>


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
	void PlayAudioLooping(const char* path, std::string name);
	void StopAudioLooping(std::string name);
	void SetVolume(float volume);
	void SetVolumeLoop(float volume, std::string name);
	float GetVolume();
};
