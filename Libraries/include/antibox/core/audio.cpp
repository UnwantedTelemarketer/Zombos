#include <iostream>
#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) { return; }

    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}

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

void AudioEngine::PlayAudioLooping(const char* path) {
    ma_result result;
    ma_decoder* currentDecoder = isPlaying1 ? &mDecoder2 : &mDecoder;
    ma_device* currentDevice = isPlaying1 ? &mDevice2 : &mDevice;

    // Initialize the decoder with the specified audio file
       result = ma_decoder_init_file(path, NULL, currentDecoder);
    if (result != MA_SUCCESS) {
        std::cout << "Failed to initialize decoder for file: " << path << std::endl;
        return;
    }
    isPlaying1 = true;

    // Enable looping for the decoder
    ma_data_source_set_looping(currentDecoder, MA_TRUE);

    // Configure the playback device
    mDeviceConfig = ma_device_config_init(ma_device_type_playback);
    mDeviceConfig.playback.format = currentDecoder->outputFormat;
    mDeviceConfig.playback.channels = currentDecoder->outputChannels;
    mDeviceConfig.sampleRate = currentDecoder->outputSampleRate;
    mDeviceConfig.dataCallback = data_callback;
    mDeviceConfig.pUserData = currentDecoder;

    // Initialize the playback device
    ma_result deviceResult = ma_device_init(NULL, &mDeviceConfig, currentDevice);
    if (deviceResult != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device." << std::endl;
        ma_decoder_uninit(currentDecoder);
        return;
    }

    // Start playback
    ma_device_start(currentDevice);
}

void AudioEngine::SetVolume(float volume)
{
    ma_engine_set_volume(&mEngine, volume);
}

float AudioEngine::GetVolume()
{
    return ma_engine_get_volume(&mEngine);
}
