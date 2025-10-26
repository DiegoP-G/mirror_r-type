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
    // outputBuffer peut être null si stream est input-only, ici on ouvre full-duplex donc on l'utilise
    int16_t *out = static_cast<int16_t *>(outputBuffer);

    if (!inputBuffer)
    {
        // pas de données entrantes : écrire du silence en sortie
        if (out)
            std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
        std::cout << "\r⚠️  Pas de données audio reçues" << std::flush;
        return paContinue;
    }

    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    // Copie directe de l'entrée vers la sortie (monitoring)
    if (out)
    {
        std::memcpy(out, in, framesPerBuffer * sizeof(int16_t));
    }

    // Calcul RMS pour l'affichage (mono)
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
        if (!info)
            continue;

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

    std::cout << "=== Test de Microphone (monitoring activé) ===" << std::endl;

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
    inputParams.channelCount = 1; // on force mono pour monitoring
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = inputInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    // Préparer les paramètres de sortie : utiliser le périphérique de sortie par défaut
    PaDeviceIndex defaultOut = Pa_GetDefaultOutputDevice();
    if (defaultOut == paNoDevice)
    {
        std::cerr << "❌ Aucun périphérique de sortie disponible!" << std::endl;
        Pa_Terminate();
        return 1;
    }
    const PaDeviceInfo *outInfo = Pa_GetDeviceInfo(defaultOut);

    PaStreamParameters outputParams;
    outputParams.device = defaultOut;
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = outInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    // Utiliser la fréquence native d'entrée pour éviter erreurs ALSA ; framesPerBuffer 256 par défaut
    double sampleRate = inputInfo->defaultSampleRate;
    unsigned long framesPerBuffer = 256;

    // Vérifier support format
    PaError testErr = Pa_IsFormatSupported(&inputParams, &outputParams, sampleRate);
    if (testErr != paFormatIsSupported)
    {
        std::cerr << "❌ Format non supporté (in/out) : " << Pa_GetErrorText(testErr) << std::endl;
        Pa_Terminate();
        return 1;
    }

    PaStream *stream;
    err = Pa_OpenStream(&stream, &inputParams, &outputParams, sampleRate, framesPerBuffer, paClipOff, micCallback,
                        nullptr);

    if (err != paNoError)
    {
        std::cerr << "❌ Impossible d'ouvrir le stream: " << Pa_GetErrorText(err) << std::endl;
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

    std::cout << "\n✓ Enregistrement et monitoring en cours... (Ctrl+C pour arrêter)" << std::endl;
    std::cout << "Parlez dans le microphone pour vous entendre et voir la barre de volume:" << std::endl;
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