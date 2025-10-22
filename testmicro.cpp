#include <algorithm>
#include <cmath>
#include <csignal>
#include <cstring>
#include <iostream>
#include <portaudio.h>
#include <vector>

volatile bool running = true;

void signalHandler(int signum)
{
    running = false;
}

int micCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    if (!inputBuffer)
    {
        std::cout << "\r⚠️  Pas de données audio reçues" << std::flush;
        return paContinue;
    }

    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    float sum = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        float sample = in[i] / 32768.0f;
        sum += sample * sample;
    }
    float rms = std::sqrt(sum / framesPerBuffer);
    float db = 20 * std::log10(rms + 0.0001f);
    float normalized = std::max(0.0f, (db + 60) / 60.0f);

    int barLength = static_cast<int>(normalized * 50);
    std::cout << "\r[";
    for (int i = 0; i < 50; i++)
    {
        if (i < barLength)
            std::cout << "\033[32m█\033[0m";
        else
            std::cout << " ";
    }
    std::cout << "] " << static_cast<int>(normalized * 100) << "% (" << static_cast<int>(db) << " dB)   " << std::flush;

    return paContinue;
}

std::vector<int> listInputDevices()
{
    std::vector<int> inputDevices;
    int numDevices = Pa_GetDeviceCount();

    std::cout << "\n🎤 Microphones disponibles:" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;

    int displayIndex = 0;
    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);

        // Afficher uniquement les périphériques avec des entrées
        if (info->maxInputChannels > 0)
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(info->hostApi);

            std::cout << "[" << displayIndex << "] ";

            // Marquer le périphérique par défaut
            if (i == Pa_GetDefaultInputDevice())
                std::cout << "⭐ ";

            std::cout << info->name << std::endl;
            std::cout << "    Backend: " << hostInfo->name << std::endl;
            std::cout << "    Canaux d'entrée: " << info->maxInputChannels << std::endl;
            std::cout << "    Fréquence: " << info->defaultSampleRate << " Hz" << std::endl;
            std::cout << "    Latence: " << (info->defaultLowInputLatency * 1000) << " ms" << std::endl;

            inputDevices.push_back(i);
            displayIndex++;
            std::cout << std::endl;
        }
    }

    if (inputDevices.empty())
    {
        std::cout << "❌ Aucun microphone détecté!" << std::endl;
    }

    return inputDevices;
}

int main()
{
    signal(SIGINT, signalHandler);

    std::cout << "=== Test de Microphone ===" << std::endl;

// Supprimer les erreurs ALSA
#ifndef _WIN32
    freopen("/dev/null", "w", stderr);
#endif

    PaError err = Pa_Initialize();

#ifndef _WIN32
    freopen("/dev/tty", "w", stderr);
#endif

    if (err != paNoError)
    {
        std::cerr << "❌ Erreur d'initialisation: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // Lister uniquement les périphériques d'entrée
    std::vector<int> inputDevices = listInputDevices();

    if (inputDevices.empty())
    {
        Pa_Terminate();
        return 1;
    }

    // Demander à l'utilisateur de choisir
    std::cout << "🎤 Choisissez un microphone (0-" << (inputDevices.size() - 1) << ") ou Entrée pour le défaut [⭐]: ";
    std::string input;
    std::getline(std::cin, input);

    PaDeviceIndex selectedDevice;
    if (input.empty())
    {
        selectedDevice = Pa_GetDefaultInputDevice();
        std::cout << "→ Utilisation du micro par défaut" << std::endl;
    }
    else
    {
        int choice = std::stoi(input);
        if (choice < 0 || choice >= inputDevices.size())
        {
            std::cerr << "❌ Choix invalide!" << std::endl;
            Pa_Terminate();
            return 1;
        }
        selectedDevice = inputDevices[choice];
    }

    if (selectedDevice == paNoDevice)
    {
        std::cerr << "❌ Aucun microphone trouvé!" << std::endl;
        Pa_Terminate();
        return 1;
    }

    const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(selectedDevice);
    const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(inputInfo->hostApi);

    std::cout << "\n✓ Microphone sélectionné:" << std::endl;
    std::cout << "  Nom: " << inputInfo->name << std::endl;
    std::cout << "  Backend: " << hostInfo->name << std::endl;
    std::cout << "  Canaux: " << inputInfo->maxInputChannels << std::endl;
    std::cout << "  Sample Rate: " << inputInfo->defaultSampleRate << " Hz" << std::endl;
    std::cout << "  Latence: " << (inputInfo->defaultLowInputLatency * 1000) << " ms" << std::endl;

    PaStreamParameters inputParams;
    inputParams.device = selectedDevice;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = inputInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream *stream;
    err = Pa_OpenStream(&stream, &inputParams, nullptr, 16000, 256, paClipOff, micCallback, nullptr);

    if (err != paNoError)
    {
        std::cerr << "❌ Impossible d'ouvrir le stream: " << Pa_GetErrorText(err) << std::endl;
        std::cerr << "   Erreur: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        std::cerr << "❌ Impossible de démarrer le stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "\n✓ Enregistrement en cours... (Ctrl+C pour arrêter)" << std::endl;
    std::cout << "Parlez dans le microphone pour voir la barre de volume:" << std::endl;
    std::cout << std::endl;

    while (running)
    {
        Pa_Sleep(100);
    }

    std::cout << "\n\n🛑 Arrêt de l'enregistrement..." << std::endl;
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}