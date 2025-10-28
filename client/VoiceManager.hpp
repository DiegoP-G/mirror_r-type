#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <portaudio.h>
#include <string>
#include <sys/types.h>
#include <vector>

struct AudioDevice
{
    int deviceIndex;
    std::string deviceName;
    int maxInputChannels;
    bool isDefaultInput;
    double defaultSampleRate;
};

class VoiceManager
{
  private:
    PaStream *inputStream = nullptr;
    PaStream *outputStream = nullptr;
    bool isRecording = false;
    int _currentInputDevice = -1;
    double _currentSampleRate = 16000.0;

    std::function<void(const std::vector<uint8_t> &)> onAudioCapture;

    static constexpr size_t RING_BUFFER_SIZE = 48000 * 4; // 4 secondes @ 48kHz
    static constexpr double NETWORK_SAMPLE_RATE = 48000.0;
    int16_t ringBuffer[RING_BUFFER_SIZE];
    std::atomic<size_t> writePos{0};
    std::atomic<size_t> readPos{0};
    std::atomic<bool> bufferInitialized{false};
    std::atomic<int> underrunCount{0};

    double outputSampleRate = 48000.0; // fréquence de sortie du périphérique

    std::vector<int16_t> resample(const std::vector<int16_t> &input, double fromRate, double toRate);

    static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                              void *userData);

    static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                            void *userData);

    size_t getAvailableSamples() const;

  public:
    VoiceManager();
    ~VoiceManager();

    std::vector<AudioDevice> getInputDevices();

    void startRecording(std::function<void(const std::vector<uint8_t> &)> callback, int deviceIndex = -1);
    void stopRecording();

    void feedAudioToRingBuffer(const std::vector<uint8_t> &audioData);

    int getCurrentInputDevice() const
    {
        return _currentInputDevice;
    }
};