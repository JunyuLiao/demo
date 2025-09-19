#!/bin/bash

set -e  # Exit on any error

echo "=== Railway Build Script ==="

# Install Python dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Install system dependencies if needed
echo "Installing system dependencies..."
if command -v apt-get &> /dev/null; then
    apt-get update
    apt-get install -y build-essential libglpk-dev
fi

# Build the C++ algorithm (Makefile will auto-detect compiler)
echo "Building C++ algorithm..."
make web-real

echo "Build completed successfully!"
