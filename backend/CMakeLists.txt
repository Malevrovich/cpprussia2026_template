cmake_minimum_required(VERSION 3.14)
project(cpp_messenger_services CXX)

# Find OpenSSL for hashing
find_package(OpenSSL REQUIRED)

# Common compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories for each service
# add_subdirectory(auth_service)
# add_subdirectory(messaging_service)
# add_subdirectory(notifications_service)
# add_subdirectory(files_service)
# add_subdirectory(reactions_service)
# add_subdirectory(status_service)