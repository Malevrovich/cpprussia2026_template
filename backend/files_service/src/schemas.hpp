#pragma once

#include <optional>
#include <string>

namespace files_service {

// V1Login - Unique user ID in the system
using V1Login = std::string;

// V1CurrentUser - Current user information
struct V1CurrentUser {
  std::optional<std::string> token;  // 128 characters if present
  V1Login login;
  std::string name;
};

// V1File - File information
struct V1File {
  V1Login login;
  std::string filename;
  std::string content;  // Base64 encoded
  std::optional<std::string> mime_type;
  std::optional<int64_t> size;
};

// V1ErrorResponse - Error response
struct V1ErrorResponse {
  std::string error;
};

// V1FileNewRequest - Request to upload a new file
struct V1FileNewRequest {
  V1Login login;
  std::string filename;
  std::string content;  // Base64 encoded
  std::optional<std::string> mime_type;
  std::optional<int64_t> size;
};

// V1FileNewResponse - Response after uploading a file
struct V1FileNewResponse {
  V1CurrentUser current_user;
  std::string uri;  // s3://bucket/path format
  V1File file;
};

// V1FileByUriRequest - Request to get a file by URI
struct V1FileByUriRequest {
  V1CurrentUser current_user;
  std::string uri;
};

// V1FileByUriResponse - Response with file information
struct V1FileByUriResponse {
  V1File file;
};

}  // namespace files_service