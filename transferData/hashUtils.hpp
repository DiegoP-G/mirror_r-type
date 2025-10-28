#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <wincrypt.h>
#include <windows.h>
#endif

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <optional>
#include <random>
#include <sstream>
#include <vector>

static constexpr int AES_KEY_BYTES = 32;
static constexpr int AES_IV_BYTES = 16;
static constexpr int TAG_BYTES = 16;

static constexpr int SALT_SIZE = 16;
static constexpr int HASH_SIZE = 32;
static constexpr int ITERATIONS = 100000;

std::string hashPassword(const std::string &password);
bool verifyPassword(const std::string &stored, const std::string &candidate);

bool generateAESKeyAndIV(unsigned char out_key[AES_KEY_BYTES], unsigned char out_iv[AES_IV_BYTES]);
EVP_PKEY *generateRSAKeyPair(int bits);

std::optional<std::vector<unsigned char>> extractPEMBytesFromRSAKeyPair(EVP_PKEY *pkey);
std::optional<EVP_PKEY *> extractPublicKeyFromPEMBytes(const std::vector<unsigned char> &pem);
std::optional<std::vector<unsigned char>> encryptBytesWithPublicKey(EVP_PKEY *pub,
                                                                    const std::vector<unsigned char> &in);
std::optional<std::vector<uint8_t>> decryptClientAesWithPublicKey(EVP_PKEY *pubKey, const unsigned char *encrypted,
                                                                  size_t encryptedLen);

std::optional<std::string> decryptAESAppendedTag(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv,
                                                 const std::vector<unsigned char> &ciphertext_with_tag);

bool aesEncryptWithTag(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv, const std::string &plaintextStr,
                       std::vector<unsigned char> &ciphertOutWithTag);

void printOpensslError(const char *ctx = nullptr);
static void handleOpensslError(const char *ctxmsg = nullptr);
