#pragma once

#include <optional>
#include <string>
#include <vector>

namespace files_service {

// V1Login - Unique user ID in the system
using V1Login = std::string;

// V1CurrentUser - Current user information
struct V1CurrentUser {
  std::string token;  // 128 characters if authorized (empty otherwise)
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

// V1FileByUriResponse - Response with file information (flat structure)
struct V1FileByUriResponse {
  V1Login login;
  std::string filename;
  std::string content;  // Base64 encoded
  std::optional<std::string> mime_type;
  std::optional<int64_t> size;
};

// V1FileMetadata - File metadata without content (for list endpoints)
struct V1FileMetadata {
  std::string uri;
  V1Login login;
  std::string filename;
  std::optional<std::string> mime_type;
  std::optional<int64_t> size;
};

// V1FileListRequest - Request to list files
struct V1FileListRequest {
  V1CurrentUser current_user;
  std::optional<V1Login> login;  // Optional filter by owner
};

// V1FileListResponse - Response with list of file metadata
struct V1FileListResponse {
  std::vector<V1FileMetadata> files;
};

}  // namespace files_service