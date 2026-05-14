cmake_minimum_required(VERSION 3.14)
project(cpp_messenger_services CXX)

# Find OpenSSL for hashing
find_package(OpenSSL REQUIRED)

# Option to use system userver or build from source
option(USERVER_FROM_SYSTEM "Use system-installed userver instead of building from source" OFF)

if(USERVER_FROM_SYSTEM)
    # Try to find userver installed in system
    find_package(userver REQUIRED)
    message(STATUS "Using system userver")
else()
    # Use userver from source (default)
    set(USERVER_ROOT /home/rexarrior/userver CACHE PATH "Path to userver sources")
    message(STATUS "Using userver from source: ${USERVER_ROOT}")
    add_subdirectory(${USERVER_ROOT} ${USERVER_ROOT}/build_debug)
endif()

# Common compiler flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add subdirectories for each service
add_subdirectory(auth_service)
# add_subdirectory(messaging_service)
# add_subdirectory(notifications_service)
# add_subdirectory(files_service)
# add_subdirectory(reactions_service)
# add_subdirectory(status_service)