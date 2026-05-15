#include "user_storage_component.hpp"
#include "crypto_utils.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>

namespace auth_service {

UserStorageComponent::UserStorageComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
  LOG_INFO() << "UserStorageComponent initialized";
}

bool UserStorageComponent::AddUser(const User& user) {
  std::lock_guard lock(mutex_);
  auto it = users_.find(user.login);
  if (it != users_.end()) {
    return false;
  }
  users_.emplace(user.login, user);
  LOG_INFO() << "User added: " << user.login;
  return true;
}

std::optional<User> UserStorageComponent::FindUser(
    const std::string& login) const {
  std::lock_guard lock(mutex_);
  auto it = users_.find(login);
  if (it == users_.end()) {
    return std::nullopt;
  }
  return it->second;
}

bool UserStorageComponent::VerifyPassword(const std::string& login,
                                          const std::string& password) {
  std::lock_guard lock(mutex_);
  auto it = users_.find(login);
  if (it == users_.end()) {
    return false;
  }
  const User& user = it->second;
  return ::auth_service::VerifyPassword(password, user.password_hash,
                                        user.salt);
}

std::string UserStorageComponent::GenerateToken() const {
  return ::auth_service::GenerateToken();
}

}  // namespace auth_service