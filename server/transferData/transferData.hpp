/* ------------------------------------------------------------------------------------ *
 *                                                                                      *
 * EPITECH PROJECT - Sat, Apr, 2025                                                     *
 * Title           - JETPACK-MIRROR                                                     *
 * Description     -                                                                    *
 *     transferData                                                                     *
 *                                                                                      *
 * ------------------------------------------------------------------------------------ *
 *                                                                                      *
 *       ▄▀▀█▄▄▄▄  ▄▀▀▄▀▀▀▄  ▄▀▀█▀▄    ▄▀▀▀█▀▀▄  ▄▀▀█▄▄▄▄  ▄▀▄▄▄▄   ▄▀▀▄ ▄▄             *
 *      ▐  ▄▀   ▐ █   █   █ █   █  █  █    █  ▐ ▐  ▄▀   ▐ █ █    ▌ █  █   ▄▀            *
 *        █▄▄▄▄▄  ▐  █▀▀▀▀  ▐   █  ▐  ▐   █       █▄▄▄▄▄  ▐ █      ▐  █▄▄▄█             *
 *        █    ▌     █          █        █        █    ▌    █         █   █             *
 *       ▄▀▄▄▄▄    ▄▀        ▄▀▀▀▀▀▄   ▄▀        ▄▀▄▄▄▄    ▄▀▄▄▄▄▀   ▄▀  ▄▀             *
 *       █    ▐   █         █       █ █          █    ▐   █     ▐   █   █               *
 *       ▐        ▐         ▐       ▐ ▐          ▐        ▐         ▐   ▐               *
 *                                                                                      *
 * ------------------------------------------------------------------------------------ */

#ifndef INCLUDED_TRANSFERDATA_HPP
#define INCLUDED_TRANSFERDATA_HPP
#include "structTransfer.hpp"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

bool handleOPCode(uint8_t opcode, bool debug = false);
void sendFrame(int socket, uint8_t opcode, const std::string& payload);
std::tuple<uint8_t, std::string> receiveFrame(int socket, std::string& buffer);

std::string serializeInt(int num);
int deserializeInt(std::string data);

std::string serializeString(const std::string& str);
std::string deserializeString(const std::string& data);

std::string serializeTuple(std::pair<int, int> tuple);
std::pair<int, int> deserializeTuple(std::string data);

#endif
