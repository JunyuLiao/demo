#!/bin/bash

# Install system dependencies for Railway
echo "Installing system dependencies..."

# Try different package managers based on the system
if command -v apt-get &> /dev/null; then
    echo "Using apt-get..."
    apt-get update
    apt-get install -y build-essential libglpk-dev clang
elif command -v yum &> /dev/null; then
    echo "Using yum..."
    yum install -y gcc-c++ glpk-devel clang
elif command -v dnf &> /dev/null; then
    echo "Using dnf..."
    dnf install -y gcc-c++ glpk-devel clang
else
    echo "No package manager found, trying to use existing tools..."
fi

# Install Python dependencies
echo "Installing Python dependencies..."
pip install -r requirements.txt

# Use Railway-compatible Makefile
echo "Using Railway-compatible Makefile..."
cp Makefile.railway Makefile

# Build the C++ algorithm
echo "Building C++ algorithm..."
make web-real

echo "Build completed successfully!"
