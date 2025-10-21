#include "hashUtils.hpp"

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
    std::cerr << "hashPassword: hash " << hashed << "\n";

    return std::string(hashed);
}

bool verifyPassword(const std::string &stored, const std::string &candidate) {
    // Hash the candidate password using the stored hash as the salt
    // std::string stored(s);
    // std::cerr << "candidate: " << candidate << "\n";

    // std::string salt = stored.substr(0, 29);
    // char *hashed = crypt(candidate.c_str(), salt.c_str());
    // std::cerr << "verifyPassword: salt " << salt << "\n";
    // if (!hashed) {
    //     std::cerr << "crypt() failed" << std::endl;
    //     return false;
    // }


    // std::cerr << "Length of hashed: " << std::strlen(hashed) << "\n";
    // std::cerr << "Length of stored: " << stored.size() << "\n";
    // std::cerr << "Raw hashed: ";
    // for (size_t i = 0; i < std::strlen(hashed); ++i) {
    //     std::cerr << static_cast<int>(hashed[i]) << " ";
    // }
    // std::cerr << "\n";

    // std::cerr << "Raw stored: ";
    // for (size_t i = 0; i < stored.size(); ++i) {
    //     std::cerr << static_cast<int>(stored[i]) << " ";
    // }
    // std::cerr << "\n";


    std::cerr << "verifyPassword: stored=" << stored << " candidate=" << candidate << "\n";
    std::cerr << "comp " << static_cast<bool>(candidate ==  stored) <<  "\n";
    // Compare the hashes
    return candidate ==  stored;
}

std::vector<uint8_t> encryptAES(const std::string &plaintext, const std::string &key, const std::string &iv)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, reinterpret_cast<const uint8_t *>(key.data()),
                           reinterpret_cast<const uint8_t *>(iv.data())) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize AES encryption");
    }

    std::vector<uint8_t> ciphertext(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, reinterpret_cast<const uint8_t *>(plaintext.data()),
                          plaintext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }
    ciphertext_len += len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize AES encryption");
    }
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

bool generateAESKeyAndIV(EVP_PKEY *localKey, EVP_PKEY *peerKey, std::string &aesKey, std::string &aesIV)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(localKey, nullptr);
    if (!ctx)
    {
        std::cerr << "Failed to create EVP_PKEY_CTX" << std::endl;
        return false;
    }

    // Derive the shared secret
    if (EVP_PKEY_derive_init(ctx) <= 0)
    {
        std::cerr << "Failed to initialize key derivation" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_derive_set_peer(ctx, peerKey) <= 0)
    {
        std::cerr << "Failed to set peer key" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    size_t secretLen = 0;
    if (EVP_PKEY_derive(ctx, nullptr, &secretLen) <= 0)
    {
        std::cerr << "Failed to determine shared secret length" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<uint8_t> sharedSecret(secretLen);
    if (EVP_PKEY_derive(ctx, sharedSecret.data(), &secretLen) <= 0)
    {
        std::cerr << "Failed to derive shared secret" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);

    // Derive AES key and IV from the shared secret
    std::vector<uint8_t> keyMaterial(EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH);
    if (!EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), nullptr, sharedSecret.data(), secretLen, 1, keyMaterial.data(),
                        keyMaterial.data() + EVP_MAX_KEY_LENGTH))
    {
        std::cerr << "Failed to derive AES key and IV" << std::endl;
        return false;
    }

    aesKey.assign(reinterpret_cast<char *>(keyMaterial.data()), EVP_MAX_KEY_LENGTH);
    aesIV.assign(reinterpret_cast<char *>(keyMaterial.data() + EVP_MAX_KEY_LENGTH), EVP_MAX_IV_LENGTH);
    return true;
}

EVP_PKEY *generateDHKeyPair()
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, nullptr);
    if (!ctx)
    {
        std::cerr << "Failed to create EVP_PKEY_CTX" << std::endl;
        return nullptr;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        std::cerr << "Failed to initialize key generation" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    if (EVP_PKEY_CTX_set_dh_paramgen_prime_len(ctx, 2048) <= 0)
    {
        std::cerr << "Failed to set DH parameter length" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY *key = nullptr;
    if (EVP_PKEY_keygen(ctx, &key) <= 0)
    {
        std::cerr << "Failed to generate DH key pair" << std::endl;
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }

    EVP_PKEY_CTX_free(ctx);
    return key;
}

std::vector<uint8_t> aesEncrypt(const std::string &plaintext, const std::string &key, const std::string &iv)
{
    // Ensure key and IV sizes are correct
    if (key.size() != 32 || iv.size() != 16)
    {
        throw std::invalid_argument("Key must be 32 bytes and IV must be 16 bytes");
    }

    // Output buffer for ciphertext
    std::vector<uint8_t> ciphertext(plaintext.size() + 16); // Add space for padding
    int len = 0, ciphertext_len = 0;

    // Perform encryption in one step
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char *>(key.data()),
                           reinterpret_cast<const unsigned char *>(iv.data())) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize AES encryption");
    }

    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                          reinterpret_cast<const unsigned char *>(plaintext.data()), plaintext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to encrypt data");
    }
    ciphertext_len += len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize AES encryption");
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    // Resize the ciphertext to the actual length
    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

std::string decryptAES(const std::vector<uint8_t> &ciphertext, const std::string &key, const std::string &iv)
{
    // Create and initialize the decryption context
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    // Initialize the decryption operation (AES-256-CBC)
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                           reinterpret_cast<const unsigned char *>(key.data()),
                           reinterpret_cast<const unsigned char *>(iv.data())) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to initialize AES decryption");
    }

    // Prepare the output buffer
    std::vector<uint8_t> plaintext(ciphertext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len = 0, plaintext_len = 0;

    // Decrypt the ciphertext
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                          ciphertext.data(), ciphertext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to decrypt data");
    }
    plaintext_len += len;

    // Finalize the decryption (handles padding)
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Failed to finalize AES decryption");
    }
    plaintext_len += len;

    // Resize the plaintext to the actual length
    plaintext.resize(plaintext_len);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    // Convert plaintext to a string and return
    return std::string(plaintext.begin(), plaintext.end());
}