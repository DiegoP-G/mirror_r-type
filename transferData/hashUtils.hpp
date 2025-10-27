#pragma once

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <crypt.h>
#include <cstring>
#include <iomanip>
#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <optional>

static constexpr int AES_KEY_BYTES = 32;
static constexpr int AES_IV_BYTES = 16;
static constexpr int TAG_BYTES = 16;

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& stored, const std::string& candidate);


bool generateAESKeyAndIV(unsigned char out_key[AES_KEY_BYTES], unsigned char out_iv[AES_IV_BYTES]);
EVP_PKEY *generateRSAKeyPair(int bits);


std::optional<std::vector<unsigned char>> extractPEMBytesFromRSAKeyPair(EVP_PKEY* pkey);
std::optional<EVP_PKEY*> extractPublicKeyFromPEMBytes(const std::vector<unsigned char>& pem);
std::optional<std::vector<unsigned char>> encryptBytesWithPublicKey(EVP_PKEY* pub, const std::vector<unsigned char>& in);
std::optional<std::vector<uint8_t>> decryptClientAesWithPublicKey(EVP_PKEY *pubKey, const unsigned char *encrypted, size_t encryptedLen);

std::optional<std::string> decryptAESAppendedTag(const std::vector<uint8_t> &key,
    const std::vector<uint8_t> &iv, const std::vector<unsigned char> &ciphertext_with_tag);

bool aesEncryptWithTag(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv,
    const std::string &plaintextStr, std::vector<unsigned char> &ciphertOutWithTag);


void printOpensslError(const char *ctx = nullptr);
static void handleOpensslError(const char *ctxmsg = nullptr);
