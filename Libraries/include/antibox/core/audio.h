#pragma once

#include "miniaudio/miniaudio.h"


class AudioEngine {
private:
	ma_engine mEngine;
	ma_device mDevice;
	ma_decoder mDecoder;
	ma_result mResult;
	ma_device_config mDeviceConfig;
public:
	ma_result init();
	void PlayAudio(const char* path);
	void PlayAudioLooping(const char* path);
	void SetVolume(float volume);
	float GetVolume();
};
