#include "hashUtils.hpp"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <wincrypt.h>
#include <windows.h>
#else
#include <crypt.h>
#include <netinet/in.h>
#endif
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <vector>

std::string toHex(const unsigned char *data, size_t len)
{
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    return oss.str();
}

std::vector<unsigned char> fromHex(const std::string &hex)
{
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

std::string hashPassword(const std::string &password)
{
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    RAND_bytes(salt, sizeof(salt));

    PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, sizeof(salt), ITERATIONS, EVP_sha256(), sizeof(hash),
                      hash);

    std::ostringstream oss;
    oss << ITERATIONS << "$" << toHex(salt, sizeof(salt)) << "$" << toHex(hash, sizeof(hash));
    return oss.str();
}

bool verifyPassword(const std::string &stored, const std::string &candidate)
{
    size_t pos1 = stored.find('$');
    size_t pos2 = stored.find('$', pos1 + 1);
    if (pos1 == std::string::npos || pos2 == std::string::npos)
        return false;

    int iterations = std::stoi(stored.substr(0, pos1));
    std::string saltHex = stored.substr(pos1 + 1, pos2 - pos1 - 1);
    std::string hashHex = stored.substr(pos2 + 1);

    std::vector<unsigned char> salt = fromHex(saltHex);
    std::vector<unsigned char> expectedHash = fromHex(hashHex);
    unsigned char computedHash[HASH_SIZE];

    PKCS5_PBKDF2_HMAC(candidate.c_str(), candidate.size(), salt.data(), salt.size(), iterations, EVP_sha256(),
                      sizeof(computedHash), computedHash);

    return CRYPTO_memcmp(computedHash, expectedHash.data(), HASH_SIZE) == 0;
}

static void handleOpensslError(const char *ctxmsg)
{
    unsigned long e = ERR_get_error();
    char buf[256];
    ERR_error_string_n(e, buf, sizeof(buf));
    if (ctxmsg)
        std::cerr << ctxmsg << ": ";
    std::cerr << "OpenSSL error: " << buf << "\n";
}

void printOpensslError(const char *ctx)
{
    unsigned long e = ERR_get_error();
    if (e == 0)
    {
        if (ctx)
            std::cerr << ctx << ": (no OpenSSL error)\n";
        return;
    }
    char buf[256];
    ERR_error_string_n(e, buf, sizeof(buf));
    if (ctx)
        std::cerr << ctx << ": ";
    std::cerr << buf << "\n";
}

bool generateAESKeyAndIV(unsigned char out_key[AES_KEY_BYTES], unsigned char out_iv[AES_IV_BYTES])
{
    if (RAND_bytes(out_key, AES_KEY_BYTES) != 1)
    {
        printOpensslError("RAND_bytes key");
        return false;
    }
    if (RAND_bytes(out_iv, AES_IV_BYTES) != 1)
    {
        printOpensslError("RAND_bytes iv");
        return false;
    }
    return true;
}

EVP_PKEY *generateRSAKeyPair(int bits)
{
    EVP_PKEY *pkey = nullptr;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    if (!ctx)
    {
        printOpensslError("EVP_PKEY_CTX_new_id");
        return nullptr;
    }
    if (EVP_PKEY_keygen_init(ctx) <= 0)
    {
        printOpensslError("EVP_PKEY_keygen_init");
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0)
    {
        printOpensslError("EVP_PKEY_CTX_set_rsa_keygen_bits");
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
    {
        printOpensslError("EVP_PKEY_keygen");
        EVP_PKEY_CTX_free(ctx);
        return nullptr;
    }
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

std::optional<std::vector<unsigned char>> extractPEMBytesFromRSAKeyPair(EVP_PKEY *pkey)
{
    BIO *mem = BIO_new(BIO_s_mem());

    if (!mem)
    {
        printOpensslError("BIO_new");
        return std::nullopt;
    }
    if (!PEM_write_bio_PUBKEY(mem, pkey))
    {
        printOpensslError("PEM_write_bio_PUBKEY");
        BIO_free(mem);
        return std::nullopt;
    }

    BUF_MEM *bptr = nullptr;
    BIO_get_mem_ptr(mem, &bptr);

    if (!bptr || bptr->length == 0)
    {
        BIO_free(mem);
        return std::nullopt;
    }
    std::vector<unsigned char> pem(bptr->data, bptr->data + bptr->length);
    BIO_free(mem);
    return pem;
}

std::optional<EVP_PKEY *> extractPublicKeyFromPEMBytes(const std::vector<unsigned char> &pem)
{
    BIO *mem = BIO_new_mem_buf(pem.data(), (int)pem.size());

    if (!mem)
    {
        printOpensslError("BIO_new_mem_buf");
        return std::nullopt;
    }
    EVP_PKEY *pub = PEM_read_bio_PUBKEY(mem, NULL, NULL, NULL);
    BIO_free(mem);
    if (!pub)
    {
        printOpensslError("PEM_read_bio_PUBKEY");
        return std::nullopt;
    }
    return pub;
}

std::optional<std::vector<unsigned char>> encryptBytesWithPublicKey(EVP_PKEY *pub, const std::vector<unsigned char> &in)
{
    if (!pub)
    {
        std::cerr << "encryptBytesWithPublicKey_checked: pub == nullptr\n";
        return std::nullopt;
    }

    // compute RSA modulus size (k) and OAEP SHA-256 limit
    size_t k = (size_t)EVP_PKEY_get_size(pub); // modulus size in bytes
    const EVP_MD *md = EVP_sha256();
    int hlen = EVP_MD_size(md); // 32 for SHA-256
    if (hlen <= 0)
    {
        std::cerr << "encryptBytesWithPublicKey_checked: failed to get hash len\n";
        return std::nullopt;
    }

    // max plaintext for OAEP with this hash
    if (k < (size_t)(2 * hlen + 2))
    {
        std::cerr << "encryptBytesWithPublicKey_checked: unexpected small RSA key k=" << k << "\n";
        return std::nullopt;
    }
    size_t max_plaintext = k - 2 * (size_t)hlen - 2;

    if (in.size() > max_plaintext)
    {
        std::cerr << "encryptBytesWithPublicKey_checked: input (" << in.size()
                  << " bytes) is too large for RSA-OAEP with this key/hash (max " << max_plaintext
                  << " bytes). Aborting.\n";
        return std::nullopt;
    }

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pub, NULL);
    if (!ctx)
    {
        handleOpensslError("EVP_PKEY_CTX_new");
        return std::nullopt;
    }
    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        handleOpensslError("EVP_PKEY_encrypt_init");
        EVP_PKEY_CTX_free(ctx);
        return std::nullopt;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        handleOpensslError("set_rsa_padding");
        EVP_PKEY_CTX_free(ctx);
        return std::nullopt;
    }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, md) <= 0)
    {
        handleOpensslError("set_oaep_md");
        EVP_PKEY_CTX_free(ctx);
        return std::nullopt;
    }

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in.data(), in.size()) <= 0)
    {
        handleOpensslError("EVP_PKEY_encrypt (size)");
        EVP_PKEY_CTX_free(ctx);
        return std::nullopt;
    }
    std::vector<unsigned char> out(outlen);
    if (EVP_PKEY_encrypt(ctx, out.data(), &outlen, in.data(), in.size()) <= 0)
    {
        handleOpensslError("EVP_PKEY_encrypt");
        EVP_PKEY_CTX_free(ctx);
        return std::nullopt;
    }
    out.resize(outlen);
    EVP_PKEY_CTX_free(ctx);
    return out;
}

bool aesEncryptWithTag(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv, const std::string &plaintextStr,
                       std::vector<unsigned char> &ciphertOutWithTag)
{
    std::vector<uint8_t> plaintext(plaintextStr.begin(), plaintextStr.end());
    unsigned char tag[TAG_BYTES];

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        handleOpensslError("EVP_CIPHER_CTX_new");
        return false;
    }
    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
    {
        handleOpensslError("EVP_EncryptInit_ex");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), NULL))
    {
        handleOpensslError("EVP_CTRL_GCM_SET_IVLEN");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    if (!EVP_EncryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()))
    {
        handleOpensslError("EVP_EncryptInit_ex (key/iv)");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    ciphertOutWithTag.resize(plaintext.size() + TAG_BYTES);
    int len = 0;
    if (!EVP_EncryptUpdate(ctx, ciphertOutWithTag.data(), &len, plaintext.data(), (int)plaintext.size()))
    {
        handleOpensslError("EVP_EncryptUpdate");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    int ciphertext_len = len;
    if (!EVP_EncryptFinal_ex(ctx, ciphertOutWithTag.data() + len, &len))
    {
        handleOpensslError("EVP_EncryptFinal_ex");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += len;
    ciphertOutWithTag.resize(ciphertext_len);
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_BYTES, tag))
    {
        handleOpensslError("EVP_CTRL_GCM_GET_TAG");
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    EVP_CIPHER_CTX_free(ctx);
    ciphertOutWithTag.insert(ciphertOutWithTag.end(), tag, tag + TAG_BYTES);
    return true;
}

static std::optional<std::string> decryptAES(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv,
                                             const std::vector<unsigned char> &ciphertextWithTag,
                                             const unsigned char tag[TAG_BYTES])
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
    {
        printOpensslError("EVP_CIPHER_CTX_new");
        return std::nullopt;
    }
    if (!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
    {
        printOpensslError("EVP_DecryptInit_ex");
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)iv.size(), NULL))
    {
        printOpensslError("EVP_CTRL_GCM_SET_IVLEN");
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    if (!EVP_DecryptInit_ex(ctx, NULL, NULL, key.data(), iv.data()))
    {
        printOpensslError("EVP_DecryptInit_ex (key/iv)");
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }

    std::vector<unsigned char> plaintext(ciphertextWithTag.size());
    int outlen = 0;
    if (!EVP_DecryptUpdate(ctx, plaintext.data(), &outlen, ciphertextWithTag.data(), (int)ciphertextWithTag.size()))
    {
        printOpensslError("EVP_DecryptUpdate");
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    int plen = outlen;

    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_BYTES, (void *)tag))
    {
        printOpensslError("EVP_CTRL_GCM_SET_TAG");
        EVP_CIPHER_CTX_free(ctx);
        return std::nullopt;
    }
    int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + outlen, &outlen);
    EVP_CIPHER_CTX_free(ctx);
    if (ret <= 0)
    {
        std::cerr << "AES-GCM tag verification failed (data tampered or wrong key/IV)\n";
        return std::nullopt;
    }
    plen += outlen;
    plaintext.resize(plen);
    std::string decryptedText(plaintext.begin(), plaintext.end());
    return decryptedText;
}

std::optional<std::string> decryptAESAppendedTag(const std::vector<uint8_t> &key, const std::vector<uint8_t> &iv,
                                                 const std::vector<unsigned char> &ciphertext_with_tag)
{
    if (ciphertext_with_tag.size() < TAG_BYTES)
    {
        std::cerr << "decryptAESAppendedTag: buffer too small to contain tag: " << ciphertext_with_tag.size() << "\n";
        return std::nullopt;
    }

    size_t ct_len = ciphertext_with_tag.size() - TAG_BYTES;

    // Split into ciphertext and tag
    std::vector<unsigned char> ciphertext(ct_len);
    memcpy(ciphertext.data(), ciphertext_with_tag.data(), ct_len);

    unsigned char tag[TAG_BYTES];
    memcpy(tag, ciphertext_with_tag.data() + ct_len, TAG_BYTES);

    return decryptAES(key, iv, ciphertext, tag);
}

std::optional<std::vector<uint8_t>> decryptClientAesWithPublicKey(EVP_PKEY *pubKey, const unsigned char *encrypted,
                                                                  size_t encryptedLen)
{
    EVP_PKEY_CTX *rsa_ctx = EVP_PKEY_CTX_new(pubKey, NULL);
    if (!rsa_ctx)
    {
        printOpensslError("EVP_PKEY_CTX_new");
        return std::nullopt;
    }
    if (EVP_PKEY_decrypt_init(rsa_ctx) <= 0)
    {
        printOpensslError("EVP_PKEY_decrypt_init");
        return std::nullopt;
    }
    if (EVP_PKEY_CTX_set_rsa_padding(rsa_ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        printOpensslError("RSA padding");
        return std::nullopt;
    }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(rsa_ctx, EVP_sha256()) <= 0)
    {
        printOpensslError("OAEP md");
        return std::nullopt;
    }

    // Get required output length
    size_t outlen = 0;
    if (EVP_PKEY_decrypt(rsa_ctx, nullptr, &outlen, encrypted, encryptedLen) <= 0)
    {
        printOpensslError("EVP_PKEY_decrypt (size)");
        return std::nullopt;
    }

    // Allocate buffer and decrypt
    std::vector<uint8_t> decrypted_aes(outlen);
    if (EVP_PKEY_decrypt(rsa_ctx, decrypted_aes.data(), &outlen, encrypted, encryptedLen) <= 0)
    {
        printOpensslError("EVP_PKEY_decrypt");
        return std::nullopt;
    }
    decrypted_aes.resize(outlen);
    EVP_PKEY_CTX_free(rsa_ctx);

    // Sanity check
    if (decrypted_aes.size() != AES_KEY_BYTES + AES_IV_BYTES)
    {
        std::cerr << "Decrypted AES key+IV size mismatch: " << decrypted_aes.size() << "\n";
        return std::nullopt;
    }
    return decrypted_aes;
}