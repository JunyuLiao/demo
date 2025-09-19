#!/bin/bash
set -e

echo "Starting Railway build process..."

# Update package lists
apt-get update

# Install build dependencies
apt-get install -y build-essential libglpk-dev python3 python3-pip

# Install Python dependencies
pip3 install -r requirements.txt

# Build the C++ algorithm
echo "Building C++ algorithm..."
make web-real

# Verify build
if [ -f "./run_web" ]; then
    echo "✓ C++ algorithm built successfully"
else
    echo "✗ C++ algorithm build failed"
    exit 1
fi

# Verify dataset
if [ -f "./datasets/car.txt" ]; then
    echo "✓ Dataset found"
else
    echo "✗ Dataset not found"
    exit 1
fi

echo "Build completed successfully"
