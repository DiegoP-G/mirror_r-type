// #include <openssl/sha.h>
// #include <openssl/rand.h>
// #include <openssl/err.h>
#include <crypt.h>
#include <cstring>
#include <iomanip>
#include <vector>
#include <iostream>

static constexpr int SALT_LEN = 16;
static constexpr int KEY_LEN = 32;
static constexpr int ITERATIONS = 200000; 
static constexpr char HEX_CHARS[] = "0123456789abcdef";

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& stored, const std::string& candidate);