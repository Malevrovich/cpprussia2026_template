"""
Common test utilities for userver services.
"""

import base64
import secrets


def generate_token() -> str:
    """
    Generate a token that mimics the auth service's token generation.

    The C++ implementation (crypto_utils.cpp) generates 96 random bytes,
    base64 encodes them without padding and without newlines, and ensures
    exactly 128 characters.

    Returns:
        A string of exactly 128 characters that can be used as a token.
    """
    # Generate 96 random bytes
    random_bytes = secrets.token_bytes(96)
    # Base64 encode without padding
    token = base64.b64encode(random_bytes).decode('ascii')
    # Remove any trailing '=' padding characters
    token = token.rstrip('=')
    # Ensure no newlines (should not happen)
    token = token.replace('\n', '').replace('\r', '')
    # Ensure exactly 128 characters (should already be 128)
    if len(token) != 128:
        # This should not happen with 96 bytes input, but pad with '=' as per C++
        if len(token) < 128:
            token += '=' * (128 - len(token))
        else:
            token = token[:128]
    return token


def generate_valid_token() -> str:
    """Alias for generate_token for backward compatibility."""
    return generate_token()


def generate_invalid_token_short() -> str:
    """Generate a token that is too short (invalid)."""
    return 'a' * 127


def generate_invalid_token_empty() -> str:
    """Generate an empty token (invalid)."""
    return ''


if __name__ == '__main__':
    # Quick test
    token = generate_token()
    print(f'Generated token: {token}')
    print(f'Length: {len(token)}')
    assert len(token) == 128, f'Token length {len(token)} != 128'