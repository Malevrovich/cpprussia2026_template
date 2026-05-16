#include "file_storage_component.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <userver/components/component.hpp>

namespace files_service {

namespace {

// Compute actual size from base64 content
int64_t ComputeActualSize(const std::string& base64_content) {
  // Base64 size formula: 3 bytes -> 4 base64 characters
  // Remove padding characters '='
  size_t len = base64_content.length();
  size_t padding = 0;
  if (len > 0 && base64_content[len - 1] == '=') {
    padding++;
    if (len > 1 && base64_content[len - 2] == '=') {
      padding++;
    }
  }
  return static_cast<int64_t>((len * 3) / 4 - padding);
}

}  // namespace

FileStorageComponent::FileStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  // Component initialization
}

std::string FileStorageComponent::StoreFile(const V1File& file) {
  std::string uri = GenerateUri();

  FileRecord record;
  record.file = file;
  record.uri = uri;
  record.created_at = std::chrono::system_clock::now();

  // Compute actual size from base64 content if not provided
  if (!record.file.size.has_value()) {
    record.file.size = ComputeActualSize(record.file.content);
  }

  {
    std::lock_guard lock(mutex_);
    files_by_uri_[uri] = std::move(record);
  }

  return uri;
}

std::optional<V1File> FileStorageComponent::GetFileByUri(
    const std::string& uri) {
  std::lock_guard lock(mutex_);
  auto it = files_by_uri_.find(uri);
  if (it == files_by_uri_.end()) {
    return std::nullopt;
  }
  return it->second.file;
}

bool FileStorageComponent::CheckOwnership(const std::string& uri,
                                          const V1Login& user_login) {
  std::lock_guard lock(mutex_);
  auto it = files_by_uri_.find(uri);
  if (it == files_by_uri_.end()) {
    return false;
  }
  return it->second.file.login == user_login;
}

FileStorageComponent::Statistics FileStorageComponent::GetStatistics() const {
  std::lock_guard lock(mutex_);
  Statistics stats{};
  stats.total_files = files_by_uri_.size();

  int64_t total_size = 0;
  for (const auto& [uri, record] : files_by_uri_) {
    if (record.file.size.has_value()) {
      total_size += record.file.size.value();
    }
  }
  stats.total_size_bytes = static_cast<size_t>(total_size);

  return stats;
}

std::string FileStorageComponent::GenerateUri() const {
  // Generate UUID for file path
  static boost::uuids::random_generator generator;
  boost::uuids::uuid id = generator();
  std::string uuid_str = boost::uuids::to_string(id);
  // Format: s3://files/{uuid}
  return "s3://files/" + uuid_str;
}

}  // namespace files_service