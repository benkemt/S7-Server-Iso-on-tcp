# Makefile for S7Server on Ubuntu/Linux
# Provides an alternative to using build_ubuntu.sh

.PHONY: all build clean install test help

# Default build type
BUILD_TYPE ?= Release

all: build

# Build the project
build:
	@echo "Building S7Server ($(BUILD_TYPE) mode)..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) .. && cmake --build .
	@echo ""
	@echo "Build complete! Executable: build/S7Server"
	@echo "Run with: sudo ./build/S7Server"

# Build in debug mode
debug:
	@$(MAKE) build BUILD_TYPE=Debug

# Build in release mode
release:
	@$(MAKE) build BUILD_TYPE=Release

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf build
	@echo "Clean complete!"

# Install the server system-wide
install: build
	@echo "Installing S7Server..."
	@cd build && sudo cmake --install .
	@echo "Installation complete!"
	@echo "Run with: sudo S7Server"

# Test the build (just verify the executable exists and runs help)
test: build
	@echo "Testing S7Server build..."
	@test -f build/S7Server && echo "✓ Executable exists" || (echo "✗ Executable not found" && exit 1)
	@ldd build/S7Server > /dev/null && echo "✓ Dependencies satisfied" || (echo "✗ Missing dependencies" && exit 1)
	@echo "Test complete!"

# Display help
help:
	@echo "S7Server Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build the project (Release mode)"
	@echo "  make build    - Build the project (use BUILD_TYPE=Debug for debug build)"
	@echo "  make debug    - Build in debug mode"
	@echo "  make release  - Build in release mode (default)"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make install  - Install system-wide (requires sudo)"
	@echo "  make test     - Test the build"
	@echo "  make help     - Display this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build in release mode"
	@echo "  make debug              # Build in debug mode"
	@echo "  make BUILD_TYPE=Debug   # Build in debug mode (alternative)"
	@echo "  make clean              # Clean build artifacts"
	@echo ""
	@echo "After building, run with: sudo ./build/S7Server"
	@echo ""
