#include <algorithm>
#include <atomic>
#include <cmath>
#include <csignal>
#include <cstring>
#include <iostream>
#include <portaudio.h>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

std::atomic<bool> running(true);
SOCKET clientSocket = INVALID_SOCKET;
sockaddr_in serverAddr;

std::atomic<int> audioSentCount(0);
std::atomic<int> audioRecvCount(0);

const double NETWORK_SAMPLE_RATE = 48000.0;
const unsigned long NETWORK_FRAMES = 480;

double inputSampleRate = 48000.0;
double outputSampleRate = 48000.0;

const size_t RING_BUFFER_SIZE = 48000 * 4;
int16_t ringBuffer[RING_BUFFER_SIZE];
std::atomic<size_t> writePos(0);
std::atomic<size_t> readPos(0);
std::atomic<bool> bufferInitialized(false);
std::atomic<int> underrunCount(0);

void signalHandler(int signum)
{
    running = false;
}

std::vector<int16_t> resample(const int16_t *input, size_t inputLen, double srcRate, double dstRate)
{
    if (srcRate == dstRate)
    {
        return std::vector<int16_t>(input, input + inputLen);
    }

    double ratio = dstRate / srcRate;
    size_t outputLen = (size_t)(inputLen * ratio);
    std::vector<int16_t> output(outputLen);

    for (size_t i = 0; i < outputLen; i++)
    {
        double srcPos = i / ratio;
        size_t srcIndex = (size_t)srcPos;

        if (srcIndex >= inputLen - 1)
        {
            output[i] = input[inputLen - 1];
        }
        else
        {
            // Interpolation linéaire
            double frac = srcPos - srcIndex;
            double sample = input[srcIndex] * (1.0 - frac) + input[srcIndex + 1] * frac;
            output[i] = (int16_t)sample;
        }
    }

    return output;
}

size_t getAvailableSamples()
{
    size_t w = writePos.load();
    size_t r = readPos.load();
    if (w >= r)
    {
        return w - r;
    }
    else
    {
        return RING_BUFFER_SIZE - r + w;
    }
}

// Callback pour l'envoi (microphone -> réseau)
int sendCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    if (!inputBuffer || !running)
    {
        return paContinue;
    }

    const int16_t *in = static_cast<const int16_t *>(inputBuffer);

    // Resampler vers 48 kHz si nécessaire
    std::vector<int16_t> resampled = resample(in, framesPerBuffer, inputSampleRate, NETWORK_SAMPLE_RATE);

    // Calcul RMS
    float sum = 0.0f;
    for (size_t i = 0; i < resampled.size(); i++)
    {
        float sample = resampled[i] / 32768.0f;
        sum += sample * sample;
    }
    float rms = std::sqrt(sum / resampled.size());

    // Envoyer les données resampleés
    size_t dataSize = resampled.size() * sizeof(int16_t);
    sendto(clientSocket, reinterpret_cast<const char *>(resampled.data()), dataSize, 0, (sockaddr *)&serverAddr,
           sizeof(serverAddr));

    audioSentCount++;

    if (rms > 0.01f && audioSentCount % 50 == 0)
    {
        float db = 20 * std::log10(rms + 0.0001f);
        std::cout << "🎤 Envoi: " << static_cast<int>(db) << " dB (" << framesPerBuffer << "→" << resampled.size()
                  << " samples)" << std::endl;
    }

    return paContinue;
}

// Callback pour la réception (buffer circulaire -> haut-parleur)
int receiveCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    int16_t *out = static_cast<int16_t *>(outputBuffer);

    if (!out)
    {
        return paContinue;
    }

    size_t available = getAvailableSamples();

    // Calculer combien de samples à 48kHz on a besoin pour produire framesPerBuffer à outputSampleRate
    size_t samplesNeeded48k = (size_t)(framesPerBuffer * (NETWORK_SAMPLE_RATE / outputSampleRate));

    // Buffer minimum: 0.2 secondes à 48 kHz
    const size_t MIN_BUFFER = 9600;
    if (!bufferInitialized && available < MIN_BUFFER)
    {
        std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
        return paContinue;
    }

    if (!bufferInitialized)
    {
        bufferInitialized = true;
        std::cout << "✓ Buffer initialisé avec " << (available * 1000 / 48000) << " ms de données" << std::endl;
    }

    if (available < samplesNeeded48k)
    {
        std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
        underrunCount++;
        if (underrunCount % 10 == 1)
        {
            std::cout << "⚠️  Buffer underrun! (besoin: " << samplesNeeded48k << ", dispo: " << available << ")"
                      << std::endl;
        }
        bufferInitialized = false;
        return paContinue;
    }

    // Lire depuis le buffer circulaire (à 48 kHz)
    std::vector<int16_t> samples48k(samplesNeeded48k);
    size_t r = readPos.load();
    for (size_t i = 0; i < samplesNeeded48k; i++)
    {
        samples48k[i] = ringBuffer[r];
        r = (r + 1) % RING_BUFFER_SIZE;
    }
    readPos.store(r);

    // Resampler vers la fréquence de sortie si nécessaire
    std::vector<int16_t> resampled =
        resample(samples48k.data(), samples48k.size(), NETWORK_SAMPLE_RATE, outputSampleRate);

    // Copier vers la sortie
    size_t copySize = std::min((size_t)framesPerBuffer, resampled.size());
    std::memcpy(out, resampled.data(), copySize * sizeof(int16_t));
    if (copySize < framesPerBuffer)
    {
        std::memset(out + copySize, 0, (framesPerBuffer - copySize) * sizeof(int16_t));
    }

    return paContinue;
}

// Thread de réception UDP
void receiveThread()
{
    char buffer[4096];

    while (running)
    {
        sockaddr_in fromAddr;
        socklen_t fromLen = sizeof(fromAddr);

#ifdef _WIN32
        DWORD timeout = 100;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
#else
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

        int recvLen = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr *)&fromAddr, &fromLen);

        if (recvLen > 0 && recvLen <= 4096 && (recvLen % sizeof(int16_t) == 0))
        {
            audioRecvCount++;

            size_t numSamples = recvLen / sizeof(int16_t);
            const int16_t *audioData = reinterpret_cast<const int16_t *>(buffer);

            // Écrire directement dans le buffer circulaire (déjà à 48 kHz)
            size_t w = writePos.load();
            for (size_t i = 0; i < numSamples; i++)
            {
                ringBuffer[w] = audioData[i];
                w = (w + 1) % RING_BUFFER_SIZE;
            }
            writePos.store(w);

            if (audioRecvCount % 100 == 0)
            {
                size_t available = getAvailableSamples();
                std::cout << "🔊 Buffer: " << available << " samples (" << (available * 1000 / 48000)
                          << " ms) | Underruns: " << underrunCount.load() << std::endl;
            }
        }
    }
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

        if (info->maxInputChannels > 0)
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(info->hostApi);

            std::cout << "[" << displayIndex << "] ";

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

std::vector<int> listOutputDevices()
{
    std::vector<int> outputDevices;
    int numDevices = Pa_GetDeviceCount();

    std::cout << "\n🔊 Périphériques de sortie disponibles:" << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;

    int displayIndex = 0;
    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (!info)
            continue;

        if (info->maxOutputChannels > 0)
        {
            const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(info->hostApi);

            std::cout << "[" << displayIndex << "] ";

            if (i == Pa_GetDefaultOutputDevice())
                std::cout << "⭐ ";

            std::cout << info->name << std::endl;
            std::cout << "    Backend: " << hostInfo->name << std::endl;
            std::cout << "    Canaux de sortie: " << info->maxOutputChannels << std::endl;
            std::cout << "    Fréquence: " << info->defaultSampleRate << " Hz" << std::endl;
            std::cout << "    Latence: " << (info->defaultLowOutputLatency * 1000) << " ms" << std::endl;

            outputDevices.push_back(i);
            displayIndex++;
            std::cout << std::endl;
        }
    }

    if (outputDevices.empty())
    {
        std::cout << "❌ Aucun périphérique de sortie détecté!" << std::endl;
    }

    return outputDevices;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signalHandler);

    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <server_ip> <port> [nom]" << std::endl;
        return 1;
    }

    std::string serverIP = argv[1];
    int port = std::atoi(argv[2]);
    std::string clientName = (argc > 3) ? argv[3] : "Client";

    std::memset(ringBuffer, 0, sizeof(ringBuffer));

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "❌ Erreur WSAStartup" << std::endl;
        return 1;
    }
#endif

    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "❌ Erreur création socket" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    std::string hello = "HELLO:" + clientName;
    sendto(clientSocket, hello.c_str(), hello.size(), 0, (sockaddr *)&serverAddr, sizeof(serverAddr));

    std::cout << "🌐 Connecté au serveur " << serverIP << ":" << port << std::endl;
    std::cout << "=== Client VoIP: " << clientName << " ===" << std::endl;

#ifndef _WIN32
    freopen("/dev/null", "w", stderr);
#endif

    PaError err = Pa_Initialize();

#ifndef _WIN32
    freopen("/dev/tty", "w", stderr);
#endif

    if (err != paNoError)
    {
        std::cerr << "❌ Erreur PortAudio: " << Pa_GetErrorText(err) << std::endl;
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::vector<int> inputDevices = listInputDevices();

    if (inputDevices.empty())
    {
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "🎤 Choisissez un microphone (0-" << (inputDevices.size() - 1) << ") ou Entrée pour le défaut [⭐]: ";
    std::string inputChoice;
    std::getline(std::cin, inputChoice);

    PaDeviceIndex inputDevice;
    if (inputChoice.empty())
    {
        inputDevice = Pa_GetDefaultInputDevice();
        std::cout << "→ Utilisation du micro par défaut" << std::endl;
    }
    else
    {
        int choice = std::stoi(inputChoice);
        if (choice < 0 || choice >= inputDevices.size())
        {
            std::cerr << "❌ Choix invalide!" << std::endl;
            Pa_Terminate();
            closesocket(clientSocket);
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        inputDevice = inputDevices[choice];
    }

    if (inputDevice == paNoDevice)
    {
        std::cerr << "❌ Aucun microphone trouvé" << std::endl;
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(inputDevice);
    inputSampleRate = inputInfo->defaultSampleRate;

    std::vector<int> outputDevices = listOutputDevices();

    if (outputDevices.empty())
    {
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    std::cout << "🔊 Choisissez un périphérique de sortie (0-" << (outputDevices.size() - 1)
              << ") ou Entrée pour le défaut [⭐]: ";
    std::string outputChoice;
    std::getline(std::cin, outputChoice);

    PaDeviceIndex outputDevice;
    if (outputChoice.empty())
    {
        outputDevice = Pa_GetDefaultOutputDevice();
        std::cout << "→ Utilisation de la sortie par défaut" << std::endl;
    }
    else
    {
        int choice = std::stoi(outputChoice);
        if (choice < 0 || choice >= outputDevices.size())
        {
            std::cerr << "❌ Choix invalide!" << std::endl;
            Pa_Terminate();
            closesocket(clientSocket);
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        outputDevice = outputDevices[choice];
    }

    if (outputDevice == paNoDevice)
    {
        std::cerr << "❌ Aucun haut-parleur trouvé" << std::endl;
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(outputDevice);
    outputSampleRate = outputInfo->defaultSampleRate;

    std::cout << "\n✓ Configuration avec resampling automatique:" << std::endl;
    std::cout << "  🎤 Microphone: " << inputInfo->name << std::endl;
    std::cout << "     Sample Rate natif: " << inputSampleRate << " Hz" << std::endl;
    std::cout << "     → Converti vers: " << NETWORK_SAMPLE_RATE << " Hz pour le réseau" << std::endl;
    std::cout << "  🔊 Sortie: " << outputInfo->name << std::endl;
    std::cout << "     Sample Rate natif: " << outputSampleRate << " Hz" << std::endl;
    std::cout << "     ← Converti depuis: " << NETWORK_SAMPLE_RATE << " Hz" << std::endl;
    std::cout << "  ✓ Resampling linéaire activé" << std::endl;

    // Calculer les frames per buffer natifs
    unsigned long inputFrames = (unsigned long)(NETWORK_FRAMES * inputSampleRate / NETWORK_SAMPLE_RATE);
    unsigned long outputFrames = (unsigned long)(NETWORK_FRAMES * outputSampleRate / NETWORK_SAMPLE_RATE);

    PaStreamParameters inputParams;
    inputParams.device = inputDevice;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = inputInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream *inputStream;
    err = Pa_OpenStream(&inputStream, &inputParams, nullptr, inputSampleRate, inputFrames, paClipOff, sendCallback,
                        nullptr);

    if (err != paNoError)
    {
        std::cerr << "❌ Erreur ouverture stream entrée: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    PaStreamParameters outputParams;
    outputParams.device = outputDevice;
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = outputInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream *outputStream;
    err = Pa_OpenStream(&outputStream, nullptr, &outputParams, outputSampleRate, outputFrames, paClipOff,
                        receiveCallback, nullptr);

    if (err != paNoError)
    {
        std::cerr << "❌ Erreur ouverture stream sortie: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(inputStream);
        Pa_Terminate();
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    Pa_StartStream(inputStream);
    Pa_StartStream(outputStream);

    std::cout << "\n✓ Communication vocale active (Ctrl+C pour quitter)" << std::endl;
    std::cout << "📊 Monitoring du buffer circulaire..." << std::endl;

    std::thread recvThread(receiveThread);

    std::thread monitorThread([&]() {
        int lastSent = 0, lastRecv = 0, lastUnderrun = 0;
        while (running)
        {
            Pa_Sleep(5000);
            if (!running)
                break;

            int sent = audioSentCount.load();
            int recv = audioRecvCount.load();
            int underruns = underrunCount.load();
            size_t available = getAvailableSamples();

            std::cout << "\n📊 Stats (5s):" << std::endl;
            std::cout << "   Envoyés: " << (sent - lastSent) << " paquets" << std::endl;
            std::cout << "   Reçus: " << (recv - lastRecv) << " paquets" << std::endl;
            std::cout << "   Buffer: " << (available * 1000 / 48000) << " ms" << std::endl;
            std::cout << "   Underruns: " << (underruns - lastUnderrun) << std::endl;

            if (sent == lastSent)
            {
                std::cout << "⚠️  Aucune capture audio - parlez plus fort!" << std::endl;
            }
            if (recv == lastRecv)
            {
                std::cout << "⚠️  Aucune réception réseau!" << std::endl;
            }
            if ((underruns - lastUnderrun) > 20)
            {
                std::cout << "❌ Trop d'underruns! Problème de réseau ou CPU?" << std::endl;
            }

            lastSent = sent;
            lastRecv = recv;
            lastUnderrun = underruns;
        }
    });

    while (running)
    {
        Pa_Sleep(100);
    }

    const char *bye = "BYE";
    sendto(clientSocket, bye, 3, 0, (sockaddr *)&serverAddr, sizeof(serverAddr));

    std::cout << "\n🛑 Arrêt de la communication..." << std::endl;

    running = false;

    recvThread.join();
    monitorThread.join();

    Pa_StopStream(inputStream);
    Pa_StopStream(outputStream);
    Pa_CloseStream(inputStream);
    Pa_CloseStream(outputStream);
    Pa_Terminate();

    closesocket(clientSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}