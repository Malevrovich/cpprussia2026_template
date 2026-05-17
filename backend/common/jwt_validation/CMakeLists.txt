add_library(jwt_validation STATIC
  jwt_validator.cpp
)

target_include_directories(jwt_validation PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(jwt_validation PUBLIC
  userver::core
)