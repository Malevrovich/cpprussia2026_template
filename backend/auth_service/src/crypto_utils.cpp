#include "crypto_utils.hpp"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <array>
#include <stdexcept>
#include <vector>

namespace auth_service {

namespace {

constexpr size_t kTokenByteLength = 96;  // 96 bytes -> 128 base64 chars

std::string BytesToHex(const unsigned char* data, size_t length) {
  static const char hex_chars[] = "0123456789abcdef";
  std::string result;
  result.reserve(length * 2);
  for (size_t i = 0; i < length; ++i) {
    result.push_back(hex_chars[(data[i] >> 4) & 0x0F]);
    result.push_back(hex_chars[data[i] & 0x0F]);
  }
  return result;
}

std::vector<unsigned char> HexToBytes(const std::string& hex) {
  if (hex.length() % 2 != 0) {
    throw std::runtime_error("Invalid hex string length");
  }
  std::vector<unsigned char> bytes;
  bytes.reserve(hex.length() / 2);
  for (size_t i = 0; i < hex.length(); i += 2) {
    unsigned char byte = (std::stoi(hex.substr(i, 2), nullptr, 16) & 0xFF);
    bytes.push_back(byte);
  }
  return bytes;
}

}  // namespace

std::string GenerateSalt(size_t length) {
  std::vector<unsigned char> salt(length);
  if (RAND_bytes(salt.data(), static_cast<int>(length)) != 1) {
    throw std::runtime_error(
        "Failed to generate cryptographically secure salt");
  }
  return BytesToHex(salt.data(), salt.size());
}

std::string HashPassword(const std::string& password, const std::string& salt,
                         unsigned int iterations) {
  constexpr size_t key_length = 32;  // SHA-256 produces 32 bytes
  std::vector<unsigned char> derived_key(key_length);

  std::vector<unsigned char> salt_bytes = HexToBytes(salt);

  if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.length()),
                        salt_bytes.data(), static_cast<int>(salt_bytes.size()),
                        static_cast<int>(iterations), EVP_sha256(),
                        static_cast<int>(key_length),
                        derived_key.data()) != 1) {
    throw std::runtime_error("PBKDF2 failed");
  }

  return BytesToHex(derived_key.data(), derived_key.size());
}

bool VerifyPassword(const std::string& password, const std::string& hash,
                    const std::string& salt, unsigned int iterations) {
  std::string computed_hash = HashPassword(password, salt, iterations);
  return computed_hash == hash;
}

std::string GenerateToken() {
  std::array<unsigned char, kTokenByteLength> random_bytes;
  if (RAND_bytes(random_bytes.data(), static_cast<int>(random_bytes.size())) !=
      1) {
    throw std::runtime_error(
        "Failed to generate cryptographically secure random bytes");
  }

  // Base64 encode without padding
  BIO* b64 = BIO_new(BIO_f_base64());
  BIO* bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);  // No newlines
  BIO_write(b64, random_bytes.data(), static_cast<int>(random_bytes.size()));
  BIO_flush(b64);

  BUF_MEM* bptr = nullptr;
  BIO_get_mem_ptr(b64, &bptr);

  std::string result(bptr->data, bptr->length);
  BIO_free_all(b64);

  // Ensure exactly 128 characters (base64 of 96 bytes should be 128 chars)
  if (result.length() != 128) {
    // This should not happen with 96 bytes input, but just in case
    result.resize(128, '=');
  }
  return result;
}

}  // namespace auth_service