#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include <vector>
#include "schemas.hpp"

namespace files_service {

// Internal file record with additional metadata
struct FileRecord {
  V1File file;
  std::string uri;  // s3://bucket/path format
  std::chrono::system_clock::time_point created_at;
};

class FileStorageComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "file-storage";

  FileStorageComponent(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  /// Store a file and generate a URI for it.
  /// Returns the generated URI (s3://files/{uuid}).
  std::string StoreFile(const V1File& file);

  /// Retrieve a file by its URI.
  /// Returns std::nullopt if file not found.
  std::optional<V1File> GetFileByUri(const std::string& uri);

  /// Check if a user is the owner of a file.
  /// Returns true if file exists and login matches.
  bool CheckOwnership(const std::string& uri, const V1Login& user_login);

  /// List files with optional filter by owner login.
  /// If login is empty, returns all files.
  std::vector<V1FileMetadata> ListFiles(
      const std::optional<V1Login>& login_filter) const;

  /// Get statistics about stored files.
  struct Statistics {
    size_t total_files;
    size_t total_size_bytes;
  };
  Statistics GetStatistics() const;

 private:
  // Generate a unique URI in s3://files/{uuid} format
  std::string GenerateUri() const;

  mutable userver::engine::Mutex mutex_;
  std::unordered_map<std::string, FileRecord>
      files_by_uri_;  // URI -> FileRecord
  std::atomic<std::uint64_t> file_counter_{0};
};

}  // namespace files_service