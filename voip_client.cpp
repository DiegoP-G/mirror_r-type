#include <algorithm>
#include <atomic>
#include <cmath>
#include <csignal>
#include <cstring>
#include <iostream>
#include <mutex>
#include <portaudio.h>
#include <queue>
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

// Buffer circulaire pour l'audio reçu
std::queue<std::vector<int16_t>> audioBuffer;
std::mutex bufferMutex;
const size_t MAX_BUFFER_SIZE = 50; // Limite pour éviter la latence excessive

void signalHandler(int signum)
{
    running = false;
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

    // Calcul RMS pour détecter si on capte vraiment du son
    float sum = 0.0f;
    for (unsigned long i = 0; i < framesPerBuffer; i++)
    {
        float sample = in[i] / 32768.0f;
        sum += sample * sample;
    }
    float rms = std::sqrt(sum / framesPerBuffer);

    size_t dataSize = framesPerBuffer * sizeof(int16_t);
    sendto(clientSocket, reinterpret_cast<const char *>(in), dataSize, 0, (sockaddr *)&serverAddr, sizeof(serverAddr));

    audioSentCount++;

    // Afficher uniquement si on détecte du son (seuil à -40dB)
    if (rms > 0.01f)
    {
        float db = 20 * std::log10(rms + 0.0001f);
        std::cout << "🎤 Micro: " << static_cast<int>(db) << " dB | Envoyé: " << dataSize << " bytes" << std::endl;
    }

    return paContinue;
}

// Callback pour la réception (buffer -> haut-parleur)
int receiveCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
    int16_t *out = static_cast<int16_t *>(outputBuffer);

    if (!out)
    {
        return paContinue;
    }

    std::lock_guard<std::mutex> lock(bufferMutex);

    if (!audioBuffer.empty())
    {
        std::vector<int16_t> data = audioBuffer.front();
        audioBuffer.pop();

        size_t samplesToCopy = std::min(data.size(), (size_t)framesPerBuffer);
        std::memcpy(out, data.data(), samplesToCopy * sizeof(int16_t));

        // Remplir le reste avec du silence si nécessaire
        if (samplesToCopy < framesPerBuffer)
        {
            std::memset(out + samplesToCopy, 0, (framesPerBuffer - samplesToCopy) * sizeof(int16_t));
        }
    }
    else
    {
        // Pas de données : silence
        std::memset(out, 0, framesPerBuffer * sizeof(int16_t));
    }

    return paContinue;
}
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
        tv.tv_usec = 100000; // 100ms
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

        int recvLen = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (sockaddr *)&fromAddr, &fromLen);

        if (recvLen > 0 && recvLen <= 4096 && (recvLen % sizeof(int16_t) == 0))
        {
            audioRecvCount++;

            size_t numSamples = recvLen / sizeof(int16_t);
            std::vector<int16_t> audioData(numSamples);
            std::memcpy(audioData.data(), buffer, recvLen);
            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                if (audioBuffer.size() < MAX_BUFFER_SIZE)
                {
                    audioBuffer.push(audioData);
                    std::cout << " Audio reçu: " << recvLen << " bytes | Buffer: " << audioBuffer.size() << " paquets"
                              << std::endl;
                }
                else
                {
                    std::cout << " Buffer plein, paquet ignoré" << std::endl;
                }
            }
        }
    }
}

std::vector<int> listInputDevices()
{
    std::vector<int> inputDevices;
    int numDevices = Pa_GetDeviceCount();

    std::cout << "\n Microphones disponibles:" << std::endl;
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
        std::cout << "Aucun microphone détecté!" << std::endl;
    }

    return inputDevices;
}

std::vector<int> listOutputDevices()
{
    std::vector<int> outputDevices;
    int numDevices = Pa_GetDeviceCount();

    std::cout << "\nPériphériques de sortie disponibles:" << std::endl;
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

    // Envoyer message de connexion
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

    // ===== SÉLECTION DU MICROPHONE =====
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

    // ===== SÉLECTION DU PÉRIPHÉRIQUE DE SORTIE =====
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

    std::cout << "\n✓ Configuration:" << std::endl;
    std::cout << "  🎤 Microphone: " << inputInfo->name << std::endl;
    std::cout << "  🔊 Sortie: " << outputInfo->name << std::endl;

    // Configuration du stream d'entrée
    PaStreamParameters inputParams;
    inputParams.device = inputDevice;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paInt16;
    inputParams.suggestedLatency = inputInfo->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream *inputStream;
    err = Pa_OpenStream(&inputStream, &inputParams, nullptr, inputInfo->defaultSampleRate, 256, paClipOff, sendCallback,
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

    // Configuration du stream de sortie
    PaStreamParameters outputParams;
    outputParams.device = outputDevice;
    outputParams.channelCount = 1;
    outputParams.sampleFormat = paInt16;
    outputParams.suggestedLatency = outputInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaStream *outputStream;
    err = Pa_OpenStream(&outputStream, nullptr, &outputParams, outputInfo->defaultSampleRate, 256, paClipOff,
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

    // Démarrer les streams
    Pa_StartStream(inputStream);
    Pa_StartStream(outputStream);

    std::cout << "\n✓ Communication vocale active (Ctrl+C pour quitter)" << std::endl;
    std::cout << "📊 Statistiques en temps réel activées..." << std::endl;

    // Lancer le thread de réception
    std::thread recvThread(receiveThread);

    // Thread de monitoring
    std::thread monitorThread([&]() {
        int lastSent = 0, lastRecv = 0;
        while (running)
        {
            Pa_Sleep(3000);
            if (!running)
                break;

            int sent = audioSentCount.load();
            int recv = audioRecvCount.load();

            std::cout << "\n📊 Stats (3s): Envoyés=" << (sent - lastSent) << " | Reçus=" << (recv - lastRecv)
                      << std::endl;

            if (sent == lastSent)
            {
                std::cout << "⚠️  Aucune capture audio - vérifiez votre microphone!" << std::endl;
            }
            if (recv == lastRecv && sent > lastSent)
            {
                std::cout << "⚠️  Aucune réception - vérifiez la connexion serveur!" << std::endl;
            }

            lastSent = sent;
            lastRecv = recv;
        }
    });

    while (running)
    {
        Pa_Sleep(100);
    }

    // Envoyer message de déconnexion
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