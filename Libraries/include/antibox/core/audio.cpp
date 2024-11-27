
#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

ma_result AudioEngine::init() {
    ma_result result;

    ma_context context;
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        return result;  // Failed to initialize the context.
    }

    result = ma_engine_init(NULL, &mEngine);
    if (result != MA_SUCCESS) {
        return result;  // Failed to initialize the engine.
    }

 
}

void AudioEngine::PlayAudio(const char* path) {
    ma_engine_play_sound(&mEngine, path, NULL);
}

void AudioEngine::SetVolume(float volume)
{
    ma_engine_set_volume(&mEngine, volume);
}

float AudioEngine::GetVolume()
{
    return ma_engine_get_volume(&mEngine);
}
