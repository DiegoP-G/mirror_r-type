#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <crypt.h>
#include <cstring>
#include <iomanip>
#include <vector>
#include <iostream>
#include <random>
#include <sstream>

static constexpr int SALT_LEN = 16;
static constexpr int KEY_LEN = 32;
static constexpr int ITERATIONS = 200000; 
static constexpr char HEX_CHARS[] = "0123456789abcdef";

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& stored, const std::string& candidate);
std::vector<uint8_t> encryptAES(const std::string &plaintext, const std::string &key, const std::string &iv);
bool generateAESKeyAndIV(EVP_PKEY *localKey, EVP_PKEY *peerKey, std::string &aesKey, std::string &aesIV);
EVP_PKEY *generateDHKeyPair();
std::string decryptAES(const std::vector<uint8_t> &ciphertext, const std::string &key, const std::string &iv);
std::vector<uint8_t> aesEncrypt(const std::string &plaintext, const std::string &key, const std::string &iv);
