#pragma once
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
    double defaultSampleRate; // ✅ Ajouter le sample rate natif
};

class VoiceManager
{
  private:
    PaStream *inputStream = nullptr;
    PaStream *outputStream = nullptr;
    bool isRecording = false;
    int _currentInputDevice = -1;
    double _currentSampleRate = 16000.0;

    std::function<void(const std::vector<u_int8_t> &)> onAudioCapture;

    std::vector<int16_t> resample(const std::vector<int16_t> &input, double fromRate, double toRate);

    static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                              void *userData);

    static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                            void *userData);

  public:
    VoiceManager();
    ~VoiceManager();

    std::vector<AudioDevice> getInputDevices();
    void startRecording(std::function<void(const std::vector<u_int8_t> &)> callback, int deviceIndex = -1);
    void stopRecording();
    void playAudio(const std::vector<u_int8_t> &audioData);

    int getCurrentInputDevice() const
    {
        return _currentInputDevice;
    }
};