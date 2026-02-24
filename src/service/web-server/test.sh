#!/bin/bash

set -e

echo "=== Testing Web Server ==="

# Change to the web-server directory
cd "$(dirname "$0")"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Build the server
echo "1. Building web-server..."
cmake ..
cmake --build .

echo "2. Build completed successfully!"

# Check if the executable exists
if [ ! -f ./web-server ]; then
    echo "Error: web-server executable not found!"
    exit 1
fi

echo "3. Web-server executable created successfully!"

# Print help information
echo "\n=== Web Server Test Summary ==="
echo "✓ Build completed successfully"
echo "✓ Executable created"
echo "\nTo run the server:"
echo "  ./web-server"
echo "\nServer will start on: http://0.0.0.0:8080"
echo "Swagger UI available at: http://localhost:8080/swagger/ui"
echo "\nAPI Endpoints:"
echo "  GET /api/drivers      - Get all drivers"
echo "  GET /api/devices      - Get all devices"
echo "  GET /api/points       - Get all points"
echo "  GET /api/alarm-rules  - Get all alarm rules"
echo "  GET /api/linkage-rules - Get all linkage rules"
echo "\n=== Test completed successfully ==="
