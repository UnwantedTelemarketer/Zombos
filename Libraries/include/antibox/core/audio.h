#pragma once

#include "miniaudio/miniaudio.h"


class AudioEngine {
private:
	ma_engine mEngine;
	ma_device mDevice, mDevice2;
	ma_decoder mDecoder, mDecoder2;
	ma_result mResult;
	ma_device_config mDeviceConfig;
	bool isPlaying1;
public:
	ma_result init();
	void PlayAudio(const char* path);
	void PlayAudioLooping(const char* path);
	void SetVolume(float volume);
	float GetVolume();
};
