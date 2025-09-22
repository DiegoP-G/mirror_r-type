/* ------------------------------------------------------------------------------------
 * *
 *                                                                                      *
 * EPITECH PROJECT - Tue, Apr, 2025 * Title           - JETPACK-MIRROR *
 * Description     - * serializers *
 *                                                                                      *
 * ------------------------------------------------------------------------------------
 * *
 *                                                                                      *
 *         ░        ░       ░░        ░        ░        ░░      ░░  ░░░░  ░ * ▒
 * ▒▒▒▒▒▒▒  ▒▒▒▒  ▒▒▒▒  ▒▒▒▒▒▒▒  ▒▒▒▒  ▒▒▒▒▒▒▒  ▒▒▒▒  ▒  ▒▒▒▒  ▒             *
 *         ▓      ▓▓▓       ▓▓▓▓▓  ▓▓▓▓▓▓▓  ▓▓▓▓      ▓▓▓  ▓▓▓▓▓▓▓        ▓ * █
 * ███████  ██████████  ███████  ████  ███████  ████  █  ████  █             *
 *         █        █  ███████        ████  ████        ██      ██  ████  █ *
 *                                                                                      *
 * ------------------------------------------------------------------------------------
 */

#include <iostream>
#include <sstream>

std::string serializeInt(int num) {
  std::stringstream ss;

  ss.write(reinterpret_cast<const char *>(&num), sizeof(num));
  std::string serializedData = ss.str();

  return serializedData;
}

int deserializeInt(std::string data) {
  std::stringstream ss(data);
  int num = 0;

  ss.read(reinterpret_cast<char *>(&num), sizeof(num));

  return num;
}

std::string serializeTuple(std::pair<int, int> tuple) {
  std::stringstream ss;

  ss.write(reinterpret_cast<const char *>(&tuple), sizeof(tuple));
  std::string serializedData = ss.str();

  return serializedData;
}

std::pair<int, int> deserializeTuple(std::string data) {
  std::stringstream ss(data);
  std::pair<int, int> tuple;

  ss.read(reinterpret_cast<char *>(&tuple), sizeof(tuple));

  return tuple;
}

std::string serializeString(const std::string &str) {
  std::stringstream ss;

  int length = static_cast<int>(str.size());
  ss.write(reinterpret_cast<const char *>(&length), sizeof(length));
  ss.write(str.data(), length);
  return ss.str();
}

std::string deserializeString(const std::string &data) {
  std::stringstream ss(data);
  int length = 0;
  ss.read(reinterpret_cast<char *>(&length), sizeof(length));

  std::string str(length, '\0');
  ss.read(&str[0], length);

  return str;
}
