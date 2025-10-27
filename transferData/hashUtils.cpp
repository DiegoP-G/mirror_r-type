#include "hashUtils.hpp"

std::string generateRandomSalt()
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "0123456789./";
    const size_t saltLength = 22;

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, sizeof(charset) - 2);

    std::ostringstream salt;
    for (size_t i = 0; i < saltLength; ++i)
    {
        salt << charset[distribution(generator)];
    }

    return salt.str();
}

std::string hashPassword(const std::string &password)
{
    std::string randomSalt = generateRandomSalt();
    std::string salt = "$2b$12$" + randomSalt;

    char *hashed = crypt(password.c_str(), salt.c_str());
    if (!hashed)
    {
        std::cerr << "crypt() failed" << std::endl;
        return "";
    }

    return std::string(hashed);
}

bool verifyPassword(const std::string &stored, const std::string &candidate)
{
    // Extract the salt from the stored hash (first 29 characters for bcrypt)
    if (stored.size() < 29)
    {
        std::cerr << "verifyPassword: stored hash is too short to contain a valid salt\n";
        return false;
    }

    std::string salt = stored.substr(0, 29);

    char *hashed = crypt(candidate.c_str(), salt.c_str());
    if (!hashed)
    {
        std::cerr << "crypt() failed\n";
        return false;
    }

    return stored == hashed;
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
    std::vector<u_int8_t> plaintext(plaintextStr.begin(), plaintextStr.end());
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