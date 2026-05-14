# Common Dockerfile for running pre-built C++ services
# Use build arguments:
#   SERVICE - service name (auth_service, messaging_service, etc.)
#   CONFIG_DIR - directory containing static_config.yaml relative to build context

ARG SERVICE=auth_service
ARG CONFIG_DIR=auth_service

FROM ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest

ARG SERVICE
ARG CONFIG_DIR

WORKDIR /app

# Copy the binary from host build directory
COPY build/${SERVICE}/${SERVICE} ./service

# Copy configuration from specified directory
COPY ${CONFIG_DIR}/static_config.yaml ./

# Expose port (should be configured in static_config.yaml)
EXPOSE 8001

CMD ["./service", "--config", "static_config.yaml"]