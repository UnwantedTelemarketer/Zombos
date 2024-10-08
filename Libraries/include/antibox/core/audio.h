#pragma once

#include "miniaudio/miniaudio.h"


class AudioEngine {
private:
	ma_engine mEngine;
public:
	ma_result init();
	void PlayAudio(const char* path);
	void SetVolume(float volume);
	float GetVolume();
};
