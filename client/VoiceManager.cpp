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
    if (!inputBuffer || !vm || !vm->onAudioCapture)
        return paContinue;

    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    std::vector<int16_t> samples16(in, in + framesPerBuffer);
    std::vector<int16_t> resampled = vm->resample(samples16, vm->_currentSampleRate, NETWORK_SAMPLE_RATE);
    float sum = 0.0f;
    for (auto sample : resampled)
    {
        float s = sample / 32768.0f;
        sum += s * s;
    }
    float rms = std::sqrt(sum / resampled.size());
    std::vector<u_int8_t> audioData(resampled.size() * sizeof(int16_t));
    std::memcpy(audioData.data(), resampled.data(), audioData.size());

    vm->onAudioCapture(audioData);

    static int sentCount = 0;
    sentCount++;
    if (rms > 0.01f && sentCount % 50 == 0)
    {
        float db = 20 * std::log10(rms + 0.0001f);
        std::cout << "🎤 [Voice] Envoi: " << static_cast<int>(db) << " dB (" << resampled.size() << " samples)"
                  << std::endl;
    }

    return paContinue;
}

void VoiceManager::feedAudioToRingBuffer(const std::vector<u_int8_t> &audioData)
{
    if (audioData.size() % sizeof(int16_t) != 0)
    {
        std::cerr << "[Voice] Invalid audio data size: " << audioData.size() << std::endl;
        return;
    }

    size_t numSamples = audioData.size() / sizeof(int16_t);
    const int16_t *samples = reinterpret_cast<const int16_t *>(audioData.data());

    size_t w = writePos.load();
    for (size_t i = 0; i < numSamples; i++)
    {
        ringBuffer[w] = samples[i];
        w = (w + 1) % RING_BUFFER_SIZE;
    }
    writePos.store(w);
}

int VoiceManager::playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                               void *userData)
{
    VoiceManager *vm = static_cast<VoiceManager *>(userData);
    int16_t *out = static_cast<int16_t *>(outputBuffer);

    if (!out)
        return paContinue;

    size_t available = vm->getAvailableSamples();

    // Calculer combien de samples 48kHz nécessaires pour produire framesPerBuffer à outputSampleRate
    size_t samplesNeeded48k = static_cast<size_t>(framesPerBuffer * (NETWORK_SAMPLE_RATE / vm->outputSampleRate));

    const size_t MIN_BUFFER = 9600;
    if (!vm->bufferInitialized && available < MIN_BUFFER)
    {
        std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
        return paContinue;
    }

    if (!vm->bufferInitialized)
    {
        vm->bufferInitialized = true;
        std::cout << "[Voice] ✓ Buffer initialisé (" << available << " samples)" << std::endl;
    }

    // Underrun check
    if (available < samplesNeeded48k)
    {
        vm->underrunCount++;
        std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
        return paContinue;
    }

    // Lire depuis ring buffer
    std::vector<int16_t> samples48k(samplesNeeded48k);
    size_t r = vm->readPos.load();
    for (size_t i = 0; i < samplesNeeded48k; i++)
    {
        samples48k[i] = vm->ringBuffer[r];
        r = (r + 1) % RING_BUFFER_SIZE;
    }
    vm->readPos.store(r);

    // Resampler vers la fréquence de sortie si nécessaire
    std::vector<int16_t> resampled = vm->resample(samples48k, NETWORK_SAMPLE_RATE, vm->outputSampleRate);

    // Copier vers la sortie
    size_t copySize = std::min(static_cast<size_t>(framesPerBuffer), resampled.size());
    std::memcpy(out, resampled.data(), copySize * sizeof(int16_t));
    if (copySize < framesPerBuffer)
    {
        std::memset(out + copySize, 0, (framesPerBuffer - copySize) * sizeof(int16_t));
    }

    return paContinue;
}

size_t VoiceManager::getAvailableSamples() const
{
    size_t w = writePos.load();
    size_t r = readPos.load();
    if (w >= r)
        return w - r;
    return RING_BUFFER_SIZE - r + w;
}

void VoiceManager::startRecording(std::function<void(const std::vector<u_int8_t> &)> callback, int deviceIndex)
{
    if (isRecording)
    {
        std::cout << "[Voice] Already recording, stopping previous session..." << std::endl;
        stopRecording();
    }

    onAudioCapture = callback;
    _currentInputDevice = (deviceIndex == -1) ? Pa_GetDefaultInputDevice() : deviceIndex;

    if (_currentInputDevice == paNoDevice)
    {
        std::cerr << "[Voice] No valid input device!" << std::endl;
        return;
    }

    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(_currentInputDevice);
    if (!deviceInfo)
    {
        std::cerr << "[Voice] Failed to get device info for device " << _currentInputDevice << std::endl;
        return;
    }

    _currentSampleRate = deviceInfo->defaultSampleRate;
    std::cout << "[Voice] Selected device: " << deviceInfo->name << " @ " << _currentSampleRate << " Hz" << std::endl;

    PaStreamParameters inputParams;
    inputParams.device = _currentInputDevice;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = deviceInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    unsigned long framesPerBuffer = static_cast<unsigned long>(_currentSampleRate * 0.03); // 30ms

    PaError err = Pa_OpenStream(&inputStream, &inputParams, nullptr, _currentSampleRate, framesPerBuffer, paClipOff,
                                recordCallback, this);

    if (err != paNoError)
    {
        std::cerr << "[Voice] Failed to open input stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    PaDeviceIndex outputDevice = Pa_GetDefaultOutputDevice();
    if (outputDevice == paNoDevice)
    {
        std::cerr << "[Voice] No default output device!" << std::endl;
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        return;
    }

    const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(outputDevice);
    outputSampleRate = outputInfo->defaultSampleRate;

    PaStreamParameters outputParams;
    outputParams.device = outputDevice;
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = outputInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    unsigned long outputFrames = static_cast<unsigned long>(outputSampleRate * 0.03); // 30ms

    err = Pa_OpenStream(&outputStream, nullptr, &outputParams, outputSampleRate, outputFrames, paClipOff, playCallback,
                        this);

    if (err != paNoError)
    {
        std::cerr << "[Voice] Failed to open output stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
        return;
    }

    Pa_StartStream(inputStream);
    Pa_StartStream(outputStream);

    isRecording = true;
    std::cout << "[Voice] ✓ Recording started (input @ " << _currentSampleRate << " Hz, output @ " << outputSampleRate
              << " Hz)" << std::endl;
}

void VoiceManager::stopRecording()
{
    if (!isRecording)
        return;

    if (inputStream)
    {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
    }

    if (outputStream)
    {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
    }

    isRecording = false;
    bufferInitialized = false;
    writePos = 0;
    readPos = 0;
    std::memset(ringBuffer, 0, sizeof(ringBuffer));

    std::cout << "[Voice] ✓ Recording stopped" << std::endl;
}

// void VoiceManager::playAudio(const std::vector<u_int8_t> &audioData)
// {
//     if (!outputStream)
//     {
//         PaStreamParameters outputParams;
//         outputParams.device = Pa_GetDefaultOutputDevice();
//         outputParams.channelCount = 1;
//         outputParams.sampleFormat = paInt16;
//         outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
//         outputParams.hostApiSpecificStreamInfo = nullptr;

//         PaError err = Pa_OpenStream(&outputStream, nullptr, &outputParams, 8000, 160, paClipOff, nullptr, nullptr);
//         if (err != paNoError)
//         {
//             std::cerr << "[Voice] Failed to open output stream: " << Pa_GetErrorText(err) << std::endl;
//             return;
//         }

//         err = Pa_StartStream(outputStream);
//         if (err != paNoError)
//         {
//             std::cerr << "[Voice] Failed to start output stream: " << Pa_GetErrorText(err) << std::endl;
//             return;
//         }

//         std::cout << "[Voice] ✓ Output stream opened @ 8000 Hz" << std::endl;
//     }

//     std::vector<int16_t> samples(audioData.size());
//     for (size_t i = 0; i < audioData.size(); i++)
//     {
//         samples[i] = static_cast<int16_t>((audioData[i] * 256) - 32768);
//     }

//     Pa_WriteStream(outputStream, samples.data(), samples.size());
// }