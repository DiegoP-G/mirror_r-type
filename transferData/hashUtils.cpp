#include "hashUtils.hpp"


// std::string toHex(const std::vector<unsigned char>& data) {
//     std::string out;
//     out.reserve(data.size() * 2);
//     for (unsigned char b : data) {
//         out.push_back(HEX_CHARS[b >> 4]);
//         out.push_back(HEX_CHARS[b & 0x0F]);
//     }
//     return out;
// }

// static inline int hexValue(char c) {
//     if (c >= '0' && c <= '9') return c - '0';
//     if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
//     if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
//     return -1;
// }

// std::vector<unsigned char> fromHex(const std::string& hex) {
//     if ((hex.size() & 1) != 0)
//         throw std::invalid_argument("hex string has odd length");

//     std::vector<unsigned char> out;
//     out.reserve(hex.size() / 2);
//     for (size_t i = 0; i < hex.size(); i += 2) {
//         int hi = hexValue(hex[i]);
//         int lo = hexValue(hex[i + 1]);
//         if (hi < 0 || lo < 0) throw std::invalid_argument("invalid hex character");
//         out.push_back(static_cast<unsigned char>((hi << 4) | lo));
//     }
//     return out;
// }

// // Constant-time comparison
// bool constantTimeCompare(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b)
// {
//     if (a.size() != b.size())
//         return false;

//     unsigned char result = 0;
//     for (size_t i = 0; i < a.size(); ++i)
//     {
//         result |= a[i] ^ b[i];
//     }
//     return result == 0;
// }

// std::string hashPassword(const std::string& password) {
//     std::vector<unsigned char> salt(SALT_LEN);
//     if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
//         std::cerr << "RAND_bytes failed" << std::endl;
//         return "";
//     }

//     std::vector<unsigned char> dk(KEY_LEN);
//     if (PKCS5_PBKDF2_HMAC(
//             password.c_str(), static_cast<int>(password.size()),
//             salt.data(), static_cast<int>(salt.size()),
//             ITERATIONS,
//             EVP_sha256(),
//             KEY_LEN,
//             dk.data()) != 1) {
//         // wipe sensitive buffers
//         OPENSSL_cleanse(salt.data(), salt.size());
//         OPENSSL_cleanse(dk.data(), dk.size());
//         std::cerr << "PKCS5_PBKDF2_HMAC failed" << std::endl;
//         return "";
//     }

//     std::string ret = "PBKDF2$sha256$" + std::to_string(ITERATIONS) + "$" + toHex(salt) + "$" + toHex(dk);

//     // optional: cleanse sensitive buffers in memory after use
//     OPENSSL_cleanse(dk.data(), dk.size());

//     return ret;
// }

// bool verifyPassword(const std::string& stored, const std::string& candidate) {
//     // parse: PBKDF2$sha256$<iterations>$<salt_hex>$<hash_hex>
//     std::istringstream ss(stored);
//     std::string token;
//     std::vector<std::string> parts;
//     while (std::getline(ss, token, '$')) parts.push_back(token);
//     if (parts.size() != 5 || parts[0] != "PBKDF2" || parts[1] != "sha256") {
//         std::cerr << "Unsupported hash format" << std::endl;
//         return false;
//     }

//     int iterations = std::stoi(parts[2]);
//     std::vector<unsigned char> salt = fromHex(parts[3]);
//     std::vector<unsigned char> expected = fromHex(parts[4]);

//     std::cerr << "DEBUG: salt bytes = " << salt.size() << ", expected bytes = " << expected.size() << "\n";

//     std::vector<unsigned char> dk(expected.size());
//     if (PKCS5_PBKDF2_HMAC(
//             candidate.c_str(), static_cast<int>(candidate.size()),
//             salt.data(), static_cast<int>(salt.size()),
//             iterations,
//             EVP_sha256(),
//             static_cast<int>(dk.size()),
//             dk.data()) != 1) {
//         OPENSSL_cleanse(dk.data(), dk.size());
//         std::cerr << "PKCS5_PBKDF2_HMAC failed at verify" << std::endl;
//         return false;
//     }

//     bool ok = constantTimeCompare(dk, expected);

//     std::cerr << "verifyPassword: stored=" << stored.c_str() << " dk=" << toHex(dk).c_str() << " expected=" << toHex(expected).c_str() << "\n";
//     OPENSSL_cleanse(dk.data(), dk.size());
//     return ok;
// }

#include <random>
#include <sstream>

std::string generateRandomSalt() {
    const char charset[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789./";
    const size_t saltLength = 22;

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

    std::ostringstream salt;
    for (size_t i = 0; i < saltLength; ++i) {
        salt << charset[distribution(generator)];
    }

    return salt.str();
}

std::string hashPassword(const std::string &password) {
    // Generate a salt for bcrypt (e.g., "$2b$12$" for bcrypt with cost 12)
    std::string randomSalt = generateRandomSalt();
    std::string salt = "$2b$12$" + randomSalt;

    std::cerr << "hashPassword: salt " << salt << "\n";

    // Hash the password using crypt
    char *hashed = crypt(password.c_str(), salt.c_str());
    if (!hashed) {
        std::cerr << "crypt() failed" << std::endl;
        return "";
    }

    return std::string(hashed);
}

bool verifyPassword(const std::string &stored, const std::string &candidate) {
    // Hash the candidate password using the stored hash as the salt
    // std::string stored(s);
    std::string salt = stored.substr(0, 29);
    char *hashed = crypt(candidate.c_str(), salt.c_str());
    std::cerr << "verifyPassword: salt " << salt << "\n";
    if (!hashed) {
        std::cerr << "crypt() failed" << std::endl;
        return false;
    }


    std::cerr << "Length of hashed: " << std::strlen(hashed) << "\n";
    std::cerr << "Length of stored: " << stored.size() << "\n";
    std::cerr << "Raw hashed: ";
    for (size_t i = 0; i < std::strlen(hashed); ++i) {
        std::cerr << static_cast<int>(hashed[i]) << " ";
    }
    std::cerr << "\n";

    std::cerr << "Raw stored: ";
    for (size_t i = 0; i < stored.size(); ++i) {
        std::cerr << static_cast<int>(stored[i]) << " ";
    }
    std::cerr << "\n";


    std::cerr << "verifyPassword: stored=" << stored << " expected=" << stored << " comp " << (hashed ==  stored) <<  "\n";
    // Compare the hashes
    return hashed ==  stored;
}