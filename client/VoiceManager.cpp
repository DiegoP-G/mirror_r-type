#include "VoiceManager.hpp"
#include <cstring>
#include <iostream>
#include <sys/types.h>

VoiceManager::VoiceManager()
{
    Pa_Initialize();
}

VoiceManager::~VoiceManager()
{
    stopRecording();
    if (inputStream)
        Pa_CloseStream(inputStream);
    if (outputStream)
        Pa_CloseStream(outputStream);
    Pa_Terminate();
}

int VoiceManager::recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                                 void *userData)
{
    VoiceManager *vm = static_cast<VoiceManager *>(userData);
    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    // Convertir 16-bit en 8-bit (effet rigolo)
    std::vector<u_int8_t> audioData(framesPerBuffer);
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        // Normaliser de [-32768, 32767] vers [0, 255]
        audioData[i] = static_cast<u_int8_t>((in[i] + 32768) / 256);
    }

    // Envoyer au callback
    if (vm->onAudioCapture)
    {
        vm->onAudioCapture(audioData);
    }

    return paContinue;
}

int VoiceManager::playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                               void *userData)
{
    // Pour la lecture, on gérera ça manuellement avec Pa_WriteStream
    return paContinue;
}

void VoiceManager::startRecording(std::function<void(const std::vector<u_int8_t> &)> callback)
{
    onAudioCapture = callback;

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    Pa_OpenStream(&inputStream, &inputParams, nullptr,
                  8000, // 8kHz sample rate
                  160,  // 20ms chunks (160 samples à 8kHz)
                  paClipOff, recordCallback, this);

    Pa_StartStream(inputStream);
    isRecording = true;
    std::cout << "[Voice] Recording started (8-bit, 8kHz)" << std::endl;
}

void VoiceManager::stopRecording()
{
    if (inputStream && isRecording)
    {
        Pa_StopStream(inputStream);
        isRecording = false;
        std::cout << "[Voice] Recording stopped" << std::endl;
    }
}

void VoiceManager::playAudio(const std::vector<u_int8_t> &audioData)
{
    if (!outputStream)
    {
        PaStreamParameters outputParams;
        outputParams.device = Pa_GetDefaultOutputDevice();
        outputParams.channelCount = 1; // Mono
        outputParams.sampleFormat = paInt16;
        outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
        outputParams.hostApiSpecificStreamInfo = nullptr;

        Pa_OpenStream(&outputStream, nullptr, &outputParams, 8000, 160, paClipOff, nullptr, nullptr);
        Pa_StartStream(outputStream);
    }

    // Convertir 8-bit en 16-bit pour la lecture
    std::vector<int16_t> samples(audioData.size());
    for (size_t i = 0; i < audioData.size(); i++)
    {
        // Dénormaliser de [0, 255] vers [-32768, 32767]
        samples[i] = static_cast<int16_t>((audioData[i] * 256) - 32768);
    }

    Pa_WriteStream(outputStream, samples.data(), samples.size());
}