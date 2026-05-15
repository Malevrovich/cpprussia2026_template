#pragma once

#include <string>
#include <string_view>

namespace auth_service {

/// Generate a cryptographically secure random salt of given byte length.
std::string GenerateSalt(size_t length = 16);

/// Hash a password with a salt using PBKDF2 with SHA-256.
/// @param password The plaintext password.
/// @param salt The salt.
/// @param iterations Number of PBKDF2 iterations (default 100000).
/// @return The derived key as hex string.
std::string HashPassword(const std::string& password, const std::string& salt,
                         unsigned int iterations = 100000);

/// Verify a password against a stored hash and salt.
/// @param password The plaintext password to verify.
/// @param hash The stored hash (hex string).
/// @param salt The stored salt.
/// @param iterations Number of PBKDF2 iterations.
/// @return True if password matches.
bool VerifyPassword(const std::string& password, const std::string& hash,
                    const std::string& salt, unsigned int iterations = 100000);

/// Generate a random token of exactly 128 characters (base64 encoded).
/// Uses cryptographically secure random bytes.
std::string GenerateToken();

}  // namespace auth_service