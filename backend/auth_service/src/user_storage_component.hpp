#pragma once

#include <optional>
#include <unordered_map>
#include <userver/components/component_base.hpp>
#include <userver/engine/mutex.hpp>
#include "crypto_utils.hpp"
#include "schemas.hpp"

namespace auth_service {

class UserStorageComponent final : public userver::components::ComponentBase {
 public:
  static constexpr std::string_view kName = "user-storage";

  UserStorageComponent(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  /// Add a new user. Returns false if user with same login already exists.
  bool AddUser(const User& user);

  /// Find user by login. Returns nullopt if not found.
  std::optional<User> FindUser(const std::string& login) const;

  /// Verify password for a given login.
  /// Returns true if user exists and password matches.
  bool VerifyPassword(const std::string& login, const std::string& password);

  /// Generate a new token for a user (does not store it).
  std::string GenerateToken() const;

 private:
  mutable userver::engine::Mutex mutex_;
  std::unordered_map<std::string, User> users_;
};

}  // namespace auth_service