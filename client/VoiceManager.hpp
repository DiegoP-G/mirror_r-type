#pragma once
#include <functional>
#include <portaudio.h>
#include <sys/types.h>
#include <vector>

class VoiceManager
{
  private:
    PaStream *inputStream = nullptr;
    PaStream *outputStream = nullptr;
    bool isRecording = false;

    std::function<void(const std::vector<u_int8_t> &)> onAudioCapture;

    static int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                              void *userData);

    static int playCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                            void *userData);

  public:
    VoiceManager();
    ~VoiceManager();

    void startRecording(std::function<void(const std::vector<u_int8_t> &)> callback);
    void stopRecording();
    void playAudio(const std::vector<u_int8_t> &audioData);
};