#!/usr/bin/env bash

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Backends to process (relative to backend directory)
BACKENDS=(
    "auth_service"
    "files_service"
    "messaging_service"
    "notifications_service"
    "reactions_service"
    "status_service"
)

# Root directory of the project
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$PROJECT_ROOT/backend"

log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $*"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
}

print_usage() {
    cat <<EOF
Usage: $0 [OPTION]

Build and test all backend services sequentially.

Options:
    --build-release      Build all backends in release mode (make build-release)
    --test-debug         Build and test all backends in debug mode (make build-debug && make test-debug)
    --help               Show this help message

Examples:
    $0 --build-release
    $0 --test-debug
EOF
}

run_make() {
    local backend="$1"
    local target="$2"
    local backend_path="$BACKEND_DIR/$backend"

    if [[ ! -d "$backend_path" ]]; then
        log_error "Backend directory not found: $backend_path"
        return 1
    fi

    log_info "Running 'make $target' in $backend..."
    cd "$backend_path"
    if make "$target"; then
        log_success "Successfully completed 'make $target' in $backend"
    else
        log_error "Failed 'make $target' in $backend"
        return 1
    fi
}

build_release_all() {
    log_info "Starting release build for all backends..."
    for backend in "${BACKENDS[@]}"; do
        run_make "$backend" "build-release" || return 1
    done
    log_success "All backends built successfully in release mode"
}

test_debug_all() {
    log_info "Starting debug build and test for all backends..."
    for backend in "${BACKENDS[@]}"; do
        run_make "$backend" "build-debug" || return 1
        run_make "$backend" "test-debug" || return 1
    done
    log_success "All backends tested successfully in debug mode"
}

main() {
    if [[ $# -eq 0 ]]; then
        print_usage
        exit 1
    fi

    case "$1" in
        --build-release)
            build_release_all
            ;;
        --test-debug)
            test_debug_all
            ;;
        --help)
            print_usage
            ;;
        *)
            log_error "Unknown option: $1"
            print_usage
            exit 1
            ;;
    esac
}

main "$@"