# Common Dockerfile for running pre-built C++ services
# Use build arguments:
#   CONFIG_DIR - directory containing static_config.yaml relative to build context
#   BINARY_PATH - path to the service binary relative to build context

ARG CONFIG_DIR=backend/auth_service/configs
ARG BINARY_PATH=backend/auth_service/build-release/auth_service

FROM ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest

ARG CONFIG_DIR
ARG BINARY_PATH

WORKDIR /app

# Copy the binary from host build directory
COPY ${BINARY_PATH} ./service

# Copy configuration from specified directory
COPY ${CONFIG_DIR}/static_config.yaml ./
COPY ${CONFIG_DIR}/config_vars.yaml ./

# Expose port (should be configured in static_config.yaml)
EXPOSE 8001

CMD ["./service", "--config", "static_config.yaml", "--config_vars", "config_vars.yaml"]