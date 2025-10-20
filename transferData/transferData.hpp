/* ------------------------------------------------------------------------------------
 * *
 *                                                                                      *
 * EPITECH PROJECT - Sat, Apr, 2025 * Title           - RTYPE-MIRROR *
 * Description     - * transferData *
 *                                                                                      *
 * ------------------------------------------------------------------------------------
 * *
 *                                                                                      *
 *       ▄▀▀█▄▄▄▄  ▄▀▀▄▀▀▀▄  ▄▀▀█▀▄    ▄▀▀▀█▀▀▄  ▄▀▀█▄▄▄▄  ▄▀▄▄▄▄   ▄▀▀▄ ▄▄ * ▐
 * ▄▀   ▐ █   █   █ █   █  █  █    █  ▐ ▐  ▄▀   ▐ █ █    ▌ █  █   ▄▀ * █▄▄▄▄▄  ▐
 * █▀▀▀▀  ▐   █  ▐  ▐   █       █▄▄▄▄▄  ▐ █      ▐  █▄▄▄█             * █    ▌
 * █          █        █        █    ▌    █         █   █             * ▄▀▄▄▄▄
 * ▄▀        ▄▀▀▀▀▀▄   ▄▀        ▄▀▄▄▄▄    ▄▀▄▄▄▄▀   ▄▀  ▄▀             * █    ▐
 * █         █       █ █          █    ▐   █     ▐   █   █               * ▐ ▐
 * ▐       ▐ ▐          ▐        ▐         ▐   ▐               *
 *                                                                                      *
 * ------------------------------------------------------------------------------------
 */

#pragma once

#include "opcode.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

bool handleOPCode(uint8_t opcode, bool debug = false);

// Will only compress if size >= minThreshold and compressed_size + header < size - margin
// Returns true if compressed and stored intoout
//bool tryCompressLZ4(const std::string &in, std::string &out, int minThreshold = 96, int margin = 8);

bool tryCompressZlib(const std::string &in, std::string &out, int minTreshold = 96, int margin = 8);

// Decompresses paylod produced by tryCompressLZ4
// Returns trye on success
//bool LZ4DecompressPayload(const std::string &in, std::string &out);

bool ZlibDecompressPayload(const std::string &in, std::string &out);

bool sendFrameTCP(int socket, uint8_t opcode, const std::string &payload);
std::tuple<uint8_t, std::string> receiveFrameTCP(int socket, std::string &buffer);

void sendFrameUDP(int sockfd, uint8_t opcode, const std::string &payload, const struct sockaddr_in &addr,
                  socklen_t addrlen);
std::tuple<uint8_t, std::string> receiveFrameUDP(int sockfd, struct sockaddr_in &addr, socklen_t &addrlen);

std::string serializeInt(int num);
int deserializeInt(std::string data);

std::string serializeString(const std::string &str);
std::string deserializeString(const std::string &data);

std::string serializeTuple(std::pair<int, int> tuple);
std::pair<int, int> deserializeTuple(std::string data);
