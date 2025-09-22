/* ------------------------------------------------------------------------------------
 * *
 *                                                                                      *
 * EPITECH PROJECT - Sat, Apr, 2025 * Title           - JETPACK-MIRROR *
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

#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

bool handleOPCode(uint8_t opcode, bool debug = false);

void sendFrameTCP(int socket, uint8_t opcode, const std::string &payload);
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
