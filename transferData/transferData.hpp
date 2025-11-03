#pragma once

#include "opcode.hpp"
#include <cstdint>
#include <string>
#include <tuple>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
// On Windows, SOCKET is already defined by winsock2.h
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
// On Unix, define SOCKET as int
typedef int SOCKET;
#endif

#include <iostream>

bool handleOPCode(uint8_t opcode, bool debug = false);

bool sendFrameTCP(SOCKET socket, uint8_t opcode, const std::string &payload);
std::tuple<uint8_t, std::string> receiveFrameTCP(SOCKET socket, std::string &buffer);

void sendFrameUDP(SOCKET sockfd, uint8_t opcode, const std::string &payload, const struct sockaddr_in &addr,
                  socklen_t addrlen);
std::tuple<uint8_t, std::string> receiveFrameUDP(SOCKET sockfd, struct sockaddr_in &addr, socklen_t &addrlen);

std::string serializeInt(int num);
int deserializeInt(std::string data);

std::string serializeString(const std::string &str);
std::string deserializeString(const std::string &data);

std::string serializeTuple(std::pair<int, int> tuple);
std::pair<int, int> deserializeTuple(std::string data);

bool tryCompressZlib(const std::string &in, std::string &out, int minTreshold = 96, int margin = 8);
bool ZlibDecompressPayload(const std::string &in, std::string &out);