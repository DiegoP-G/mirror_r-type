#include "VoiceManager.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <sys/types.h>

VoiceManager::VoiceManager()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        std::cerr << "[Voice] Failed to initialize PortAudio: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    std::cout << "[Voice] PortAudio initialized successfully" << std::endl;
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

std::vector<AudioDevice> VoiceManager::getInputDevices()
{
    std::vector<AudioDevice> devices;

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0)
    {
        std::cerr << "[Voice] Error getting device count: " << Pa_GetErrorText(numDevices) << std::endl;
        return devices;
    }

    PaDeviceIndex defaultInput = Pa_GetDefaultInputDevice();

    std::cout << "[Voice] Found " << numDevices << " audio devices" << std::endl;

    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);

        if (deviceInfo && deviceInfo->maxInputChannels > 0)
        {
            AudioDevice device;
            device.deviceIndex = i;
            device.deviceName = deviceInfo->name;
            device.maxInputChannels = deviceInfo->maxInputChannels;
            device.isDefaultInput = (i == defaultInput);
            device.defaultSampleRate = deviceInfo->defaultSampleRate; // ✅ Stocker le sample rate natif

            devices.push_back(device);

            std::cout << "[Voice] Input Device " << i << ": " << device.deviceName << " @ " << device.defaultSampleRate
                      << " Hz" << (device.isDefaultInput ? " (DEFAULT)" : "") << std::endl;
        }
    }

    return devices;
}

// ✅ Rééchantillonnage simple par interpolation linéaire
std::vector<int16_t> VoiceManager::resample(const std::vector<int16_t> &input, double fromRate, double toRate)
{
    if (fromRate == toRate)
        return input;

    double ratio = fromRate / toRate;
    size_t outputSize = static_cast<size_t>(input.size() / ratio);
    std::vector<int16_t> output(outputSize);

    for (size_t i = 0; i < outputSize; i++)
    {
        double srcIndex = i * ratio;
        size_t index1 = static_cast<size_t>(srcIndex);
        size_t index2 = std::min(index1 + 1, input.size() - 1);

        double frac = srcIndex - index1;
        output[i] = static_cast<int16_t>(input[index1] * (1.0 - frac) + input[index2] * frac);
    }

    return output;
}

int VoiceManager::recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                                 void *userData)
{
    VoiceManager *vm = static_cast<VoiceManager *>(userData);
    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    if (inputBuffer == nullptr)
    {
        std::cerr << "[Voice] Null input buffer in callback!" << std::endl;
        return paContinue;
    }

    // ✅ Convertir en vecteur 16-bit
    std::vector<int16_t> samples16(in, in + framesPerBuffer);

    // ✅ Rééchantillonner de la fréquence native vers 8000 Hz
    std::vector<int16_t> resampled = vm->resample(samples16, vm->_currentSampleRate, 8000.0);

    // ✅ Convertir en 8-bit pour la transmission
    std::vector<u_int8_t> audioData(resampled.size());
    for (size_t i = 0; i < resampled.size(); i++)
    {
        audioData[i] = static_cast<u_int8_t>((resampled[i] + 32768) / 256);
    }

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
    return paContinue;
}

void VoiceManager::startRecording(std::function<void(const std::vector<u_int8_t> &)> callback, int deviceIndex)
{
    onAudioCapture = callback;

    if (deviceIndex == -1)
    {
        deviceIndex = Pa_GetDefaultInputDevice();
    }

    if (deviceIndex == paNoDevice)
    {
        std::cerr << "[Voice] No input device available!" << std::endl;
        return;
    }

    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    if (!deviceInfo)
    {
        std::cerr << "[Voice] Invalid device index: " << deviceIndex << std::endl;
        return;
    }

    // ✅ Utiliser le sample rate natif du périphérique
    _currentSampleRate = deviceInfo->defaultSampleRate;

    std::cout << "[Voice] Starting recording with device: " << deviceInfo->name << std::endl;
    std::cout << "[Voice] Native sample rate: " << _currentSampleRate << " Hz (will resample to 8000 Hz)" << std::endl;

    PaStreamParameters inputParams;
    inputParams.device = deviceIndex;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = deviceInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    // ✅ Calculer framesPerBuffer en fonction du sample rate natif
    unsigned long framesPerBuffer = static_cast<unsigned long>(_currentSampleRate * 0.03); // 30ms de buffer

    // ✅ Ouvrir le stream avec le sample rate natif
    PaError err = Pa_OpenStream(&inputStream, &inputParams, nullptr, _currentSampleRate, framesPerBuffer, paClipOff,
                                recordCallback, this);

    if (err != paNoError)
    {
        std::cerr << "[Voice] Failed to open input stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    err = Pa_StartStream(inputStream);
    if (err != paNoError)
    {
        std::cerr << "[Voice] Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        return;
    }

    isRecording = true;
    _currentInputDevice = deviceIndex;
    std::cout << "[Voice] ✓ Recording started on device " << deviceIndex << " @ " << _currentSampleRate << " Hz"
              << std::endl;
}

void VoiceManager::stopRecording()
{
    if (inputStream && isRecording)
    {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        isRecording = false;
        _currentInputDevice = -1;
        std::cout << "[Voice] Recording stopped" << std::endl;
    }
}

void VoiceManager::playAudio(const std::vector<u_int8_t> &audioData)
{
    if (!outputStream)
    {
        PaStreamParameters outputParams;
        outputParams.device = Pa_GetDefaultOutputDevice();
        outputParams.channelCount = 1;
        outputParams.sampleFormat = paInt16;
        outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
        outputParams.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_OpenStream(&outputStream, nullptr, &outputParams, 8000, 160, paClipOff, nullptr, nullptr);
        if (err != paNoError)
        {
            std::cerr << "[Voice] Failed to open output stream: " << Pa_GetErrorText(err) << std::endl;
            return;
        }

        err = Pa_StartStream(outputStream);
        if (err != paNoError)
        {
            std::cerr << "[Voice] Failed to start output stream: " << Pa_GetErrorText(err) << std::endl;
            return;
        }

        std::cout << "[Voice] ✓ Output stream opened @ 8000 Hz" << std::endl;
    }

    std::vector<int16_t> samples(audioData.size());
    for (size_t i = 0; i < audioData.size(); i++)
    {
        samples[i] = static_cast<int16_t>((audioData[i] * 256) - 32768);
    }

    Pa_WriteStream(outputStream, samples.data(), samples.size());
}