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

void AudioEngine::StopAudioLooping(std::string name) {
    ma_device_stop(mDevices[name]);
}

void AudioEngine::PlayAudioLooping(std::string path, std::string name) {

    if (mDevices.count(name) != 0) {
        ma_device_uninit(mDevices[name]); // Uninitialize and clean up the device.
        delete mDevices[name];           // Free the allocated memory.
        mDevices.erase(name);            // Remove from the map.
    }

    if (mDecoders.count(name) != 0) {
        ma_decoder_uninit(mDecoders[name]); // Uninitialize and clean up the decoder.
        delete mDecoders[name];             // Free the allocated memory.
        mDecoders.erase(name);              // Remove from the map.
    }

    ma_result result;
    ma_decoder* currentDecoder = new ma_decoder;
    ma_device* currentDevice = new ma_device;
    mDecoders.insert({ name, currentDecoder });
    mDevices.insert({ name, currentDevice });

    // Initialize the decoder with the specified audio file
    result = ma_decoder_init_file(path.c_str(), NULL, currentDecoder);
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

void AudioEngine::SetVolumeLoop(float volume, std::string name)
{
    // Check if the device exists for the given name
    if (mDevices.count(name) == 0) {
        std::cout << "this device does not exist." << std::endl;
        return;
    }

    if (mDevices[name] == nullptr) {
        std::cout << "this device is in the map but is a nullptr." << std::endl;
        return;  // Or handle this error more gracefully
    }

    // Ensure the volume is within a valid range (0.0f to 1.0f)
    if (volume < 0.0f || volume > 1.0f) {
        return;
    }

    // Set the volume for the specified device
    ma_result result = ma_device_set_master_volume(mDevices[name], volume);
    if (result != MA_SUCCESS) {
        throw std::runtime_error("Failed to set volume for the specified audio track.");
    }
}

float AudioEngine::GetVolume()
{
    return ma_engine_get_volume(&mEngine);
}
